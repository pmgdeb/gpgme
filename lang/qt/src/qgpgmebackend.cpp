/*
    qgpgmebackend.cpp

    This file is part of qgpgme, the Qt API binding for gpgme
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB
    Copyright (c) 2016 Intevation GmbH

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "qgpgmebackend.h"

#include "qgpgmenewcryptoconfig.h"

#include "qgpgmekeygenerationjob.h"
#include "qgpgmekeylistjob.h"
#include "qgpgmelistallkeysjob.h"
#include "qgpgmedecryptjob.h"
#include "qgpgmedecryptverifyjob.h"
#include "qgpgmerefreshkeysjob.h"
#include "qgpgmedeletejob.h"
#include "qgpgmesecretkeyexportjob.h"
#include "qgpgmedownloadjob.h"
#include "qgpgmesignencryptjob.h"
#include "qgpgmeencryptjob.h"
#include "qgpgmesignjob.h"
#include "qgpgmesignkeyjob.h"
#include "qgpgmeexportjob.h"
#include "qgpgmeverifydetachedjob.h"
#include "qgpgmeimportjob.h"
#include "qgpgmeimportfromkeyserverjob.h"
#include "qgpgmeverifyopaquejob.h"
#include "qgpgmechangeexpiryjob.h"
#include "qgpgmechangeownertrustjob.h"
#include "qgpgmechangepasswdjob.h"
#include "qgpgmeadduseridjob.h"

#include "error.h"
#include "engineinfo.h"

#include <QFile>
#include <QString>

const char QGpgME::QGpgMEBackend::OpenPGP[] = "OpenPGP";
const char QGpgME::QGpgMEBackend::SMIME[] = "SMIME";

namespace
{

class Protocol : public QGpgME::Protocol
{
    GpgME::Protocol mProtocol;
public:
    explicit Protocol(GpgME::Protocol proto) : mProtocol(proto) {}

    QString name() const Q_DECL_OVERRIDE
    {
        switch (mProtocol) {
        case GpgME::OpenPGP: return QStringLiteral("OpenPGP");
        case GpgME::CMS:     return QStringLiteral("SMIME");
        default:             return QString();
        }
    }

    QString displayName() const Q_DECL_OVERRIDE
    {
        // ah (2.4.16): Where is this used and isn't this inverted
        // with name
        switch (mProtocol) {
        case GpgME::OpenPGP: return QStringLiteral("gpg");
        case GpgME::CMS:     return QStringLiteral("gpgsm");
        default:             return QStringLiteral("unknown");
        }
    }

    QGpgME::SpecialJob *specialJob(const char *, const QMap<QString, QVariant> &) const Q_DECL_OVERRIDE
    {
        return 0;
    }

    QGpgME::KeyListJob *keyListJob(bool remote, bool includeSigs, bool validate) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        unsigned int mode = context->keyListMode();
        if (remote) {
            mode |= GpgME::Extern;
            mode &= ~GpgME::Local;
        } else {
            mode |= GpgME::Local;
            mode &= ~GpgME::Extern;
        }
        if (includeSigs) {
            mode |= GpgME::Signatures;
        }
        if (validate) {
            mode |= GpgME::Validate;
        }
        context->setKeyListMode(mode);
        return new QGpgME::QGpgMEKeyListJob(context);
    }

    QGpgME::ListAllKeysJob *listAllKeysJob(bool includeSigs, bool validate) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        unsigned int mode = context->keyListMode();
        mode |= GpgME::Local;
        mode &= ~GpgME::Extern;
        if (includeSigs) {
            mode |= GpgME::Signatures;
        }
        if (validate) {
            mode |= GpgME::Validate;
            /* Setting the context to offline mode disables CRL / OCSP checks in
               this Job. Otherwise we would try to fetch the CRL's for all CMS
               keys in the users keyring because GpgME::Validate includes remote
               resources by default in the validity check.
               This setting only has any effect if gpgsm >= 2.1.6 is used.
               */
            context->setOffline(true);
        }
        context->setKeyListMode(mode);
        return new QGpgME::QGpgMEListAllKeysJob(context);
    }

    QGpgME::EncryptJob *encryptJob(bool armor, bool textmode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        context->setTextMode(textmode);
        return new QGpgME::QGpgMEEncryptJob(context);
    }

    QGpgME::DecryptJob *decryptJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEDecryptJob(context);
    }

    QGpgME::SignJob *signJob(bool armor, bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        context->setTextMode(textMode);
        return new QGpgME::QGpgMESignJob(context);
    }

    QGpgME::VerifyDetachedJob *verifyDetachedJob(bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setTextMode(textMode);
        return new QGpgME::QGpgMEVerifyDetachedJob(context);
    }

    QGpgME::VerifyOpaqueJob *verifyOpaqueJob(bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setTextMode(textMode);
        return new QGpgME::QGpgMEVerifyOpaqueJob(context);
    }

    QGpgME::KeyGenerationJob *keyGenerationJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEKeyGenerationJob(context);
    }

    QGpgME::ImportJob *importJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEImportJob(context);
    }

    QGpgME::ImportFromKeyserverJob *importFromKeyserverJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEImportFromKeyserverJob(context);
    }

    QGpgME::ExportJob *publicKeyExportJob(bool armor) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        return new QGpgME::QGpgMEExportJob(context);
    }

    QGpgME::ExportJob *secretKeyExportJob(bool armor, const QString &charset) const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::CMS) { // fixme: add support for gpg, too
            return 0;
        }

        // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
        return new QGpgME::QGpgMESecretKeyExportJob(armor, charset);
    }

    QGpgME::RefreshKeysJob *refreshKeysJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::CMS) { // fixme: add support for gpg, too
            return 0;
        }

        // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
        return new QGpgME::QGpgMERefreshKeysJob();
    }

    QGpgME::DownloadJob *downloadJob(bool armor) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        // this is the hackish interface for downloading from keyserers currently:
        context->setKeyListMode(GpgME::Extern);
        return new QGpgME::QGpgMEDownloadJob(context);
    }

    QGpgME::DeleteJob *deleteJob() const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEDeleteJob(context);
    }

    QGpgME::SignEncryptJob *signEncryptJob(bool armor, bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setArmor(armor);
        context->setTextMode(textMode);
        return new QGpgME::QGpgMESignEncryptJob(context);
    }

    QGpgME::DecryptVerifyJob *decryptVerifyJob(bool textMode) const Q_DECL_OVERRIDE
    {
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }

        context->setTextMode(textMode);
        return new QGpgME::QGpgMEDecryptVerifyJob(context);
    }

    QGpgME::ChangeExpiryJob *changeExpiryJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEChangeExpiryJob(context);
    }

    QGpgME::ChangePasswdJob *changePasswdJob() const Q_DECL_OVERRIDE
    {
        if (!GpgME::hasFeature(GpgME::PasswdFeature, 0)) {
            return 0;
        }
        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEChangePasswdJob(context);
    }

    QGpgME::SignKeyJob *signKeyJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMESignKeyJob(context);
    }

    QGpgME::ChangeOwnerTrustJob *changeOwnerTrustJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEChangeOwnerTrustJob(context);
    }

    QGpgME::AddUserIDJob *addUserIDJob() const Q_DECL_OVERRIDE
    {
        if (mProtocol != GpgME::OpenPGP) {
            return 0;    // only supported by gpg
        }

        GpgME::Context *context = GpgME::Context::createForProtocol(mProtocol);
        if (!context) {
            return 0;
        }
        return new QGpgME::QGpgMEAddUserIDJob(context);
    }

};

}

QGpgME::QGpgMEBackend::QGpgMEBackend()
    : mCryptoConfig(0),
      mOpenPGPProtocol(0),
      mSMIMEProtocol(0)
{
    GpgME::initializeLibrary();
}

QGpgME::QGpgMEBackend::~QGpgMEBackend()
{
    delete mCryptoConfig; mCryptoConfig = 0;
    delete mOpenPGPProtocol; mOpenPGPProtocol = 0;
    delete mSMIMEProtocol; mSMIMEProtocol = 0;
}

QString QGpgME::QGpgMEBackend::name() const
{
    return QStringLiteral("gpgme");
}

QString QGpgME::QGpgMEBackend::displayName() const
{
    return QStringLiteral("GpgME");
}

QGpgME::CryptoConfig *QGpgME::QGpgMEBackend::config() const
{
    if (!mCryptoConfig) {
        if (GpgME::hasFeature(GpgME::GpgConfEngineFeature, 0)) {
            mCryptoConfig = new QGpgMENewCryptoConfig;
        }
    }
    return mCryptoConfig;
}

static bool check(GpgME::Protocol proto, QString *reason)
{
    if (!GpgME::checkEngine(proto)) {
        return true;
    }
    if (!reason) {
        return false;
    }
    // error, check why:
#if 0
Port away from localised string or delete.
    const GpgME::EngineInfo ei = GpgME::engineInfo(proto);
    if (ei.isNull()) {
        *reason = i18n("GPGME was compiled without support for %1.", proto == GpgME::CMS ? QLatin1String("S/MIME") : QLatin1String("OpenPGP"));
    } else if (ei.fileName() && !ei.version()) {
        *reason = i18n("Engine %1 is not installed properly.", QFile::decodeName(ei.fileName()));
    } else if (ei.fileName() && ei.version() && ei.requiredVersion())
        *reason = i18n("Engine %1 version %2 installed, "
                       "but at least version %3 is required.",
                       QFile::decodeName(ei.fileName()), QLatin1String(ei.version()), QLatin1String(ei.requiredVersion()));
    else {
        *reason = i18n("Unknown problem with engine for protocol %1.", proto == GpgME::CMS ? QLatin1String("S/MIME") : QLatin1String("OpenPGP"));
    }
#endif
    return false;
}

bool QGpgME::QGpgMEBackend::checkForOpenPGP(QString *reason) const
{
    return check(GpgME::OpenPGP, reason);
}

bool QGpgME::QGpgMEBackend::checkForSMIME(QString *reason) const
{
    return check(GpgME::CMS, reason);
}

bool QGpgME::QGpgMEBackend::checkForProtocol(const char *name, QString *reason) const
{
    if (qstricmp(name, OpenPGP) == 0) {
        return check(GpgME::OpenPGP, reason);
    }
    if (qstricmp(name, SMIME) == 0) {
        return check(GpgME::CMS, reason);
    }
    if (reason) {
        *reason = QStringLiteral("Unsupported protocol \"%1\"").arg(QLatin1String(name));
    }
    return false;
}

QGpgME::Protocol *QGpgME::QGpgMEBackend::openpgp() const
{
    if (!mOpenPGPProtocol)
        if (checkForOpenPGP()) {
            mOpenPGPProtocol = new ::Protocol(GpgME::OpenPGP);
        }
    return mOpenPGPProtocol;
}

QGpgME::Protocol *QGpgME::QGpgMEBackend::smime() const
{
    if (!mSMIMEProtocol)
        if (checkForSMIME()) {
            mSMIMEProtocol = new ::Protocol(GpgME::CMS);
        }
    return mSMIMEProtocol;
}

QGpgME::Protocol *QGpgME::QGpgMEBackend::protocol(const char *name) const
{
    if (qstricmp(name, OpenPGP) == 0) {
        return openpgp();
    }
    if (qstricmp(name, SMIME) == 0) {
        return smime();
    }
    return 0;
}

bool QGpgME::QGpgMEBackend::supportsProtocol(const char *name) const
{
    return qstricmp(name, OpenPGP) == 0 || qstricmp(name, SMIME) == 0;
}

const char *QGpgME::QGpgMEBackend::enumerateProtocols(int i) const
{
    switch (i) {
    case 0: return OpenPGP;
    case 1: return SMIME;
    default: return 0;
    }
}