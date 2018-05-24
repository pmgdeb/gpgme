/* gpgme.js - Javascript integration for gpgme
 * Copyright (C) 2018 Bundesamt für Sicherheit in der Informationstechnik
 *
 * This file is part of GPGME.
 *
 * GPGME is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * GPGME is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 * SPDX-License-Identifier: LGPL-2.1+
 */

const err_list = {
    // Connection
    'CONN_NO_CONNECT': {
        msg:'Connection with the nativeMessaging host could not be'
            + ' established.',
        type: 'error'
    },
    'CONN_DISCONNECTED': {
        msg:'Connection with the nativeMessaging host was lost.',
        type: 'error'
    },
    'CONN_EMPTY_GPG_ANSWER':{
        msg: 'The nativeMessaging answer was empty.',
        type: 'error'
    },
    'CONN_TIMEOUT': {
        msg: 'A connection timeout was exceeded.',
        type: 'error'
    },
    'CONN_UNEXPECTED_ANSWER': {
        msg: 'The answer from gnupg was not as expected.',
        type: 'error'
    },
    'CONN_ALREADY_CONNECTED':{
        msg: 'A connection was already established.',
        type: 'warning'
    },
    // Message/Data
    'MSG_INCOMPLETE': {
        msg: 'The Message did not match the minimum requirements for'
            + ' the interaction.',
        type: 'error'
    },
    'MSG_EMPTY' : {
        msg: 'The Message is empty.',
        type: 'error'
    },
    'MSG_WRONG_OP': {
        msg: 'The operation requested could not be found',
        type: 'error'
    },
    'MSG_NO_KEYS' : {
        msg: 'There were no valid keys provided.',
        type: 'warning'
    },
    'MSG_NOT_A_FPR': {
        msg: 'The String is not an accepted fingerprint',
        type: 'warning'
    },
    'KEY_INVALID': {
        msg:'Key object is invalid',
        type: 'error'
    },
    // generic
    'PARAM_WRONG':{
        msg: 'Invalid parameter was found',
        type: 'error'
    },
    'PARAM_IGNORED': {
        msg: 'An parameter was set that has no effect in gpgmejs',
        type: 'warning'
    },
    'GENERIC_ERROR': {
        msg: 'Unspecified error',
        type: 'error'
    }
};

/**
 * Checks the given error code and returns an error object with some
 * information about meaning and origin
 * @param {*} code Error code. Should be in err_list or 'GNUPG_ERROR'
 * @param {*} info Error message passed through if code is 'GNUPG_ERROR'
 */
export function gpgme_error(code = 'GENERIC_ERROR', info){
    if (err_list.hasOwnProperty(code)){
        if (err_list[code].type === 'error'){
            return new GPGME_Error(code);
        }
        if (err_list[code].type === 'warning'){
            console.warn(code + ': ' + err_list[code].msg);
        }
        return null;
    } else if (code === 'GNUPG_ERROR'){
        return new GPGME_Error(code, info);
    }
    else {
        return new GPGME_Error('GENERIC_ERROR');
    }
}

class GPGME_Error extends Error{
    constructor(code, msg=''){
        if (code === 'GNUPG_ERROR' && typeof(msg) === 'string'){
            super(msg);
        } else if (err_list.hasOwnProperty(code)){
            super(err_list[code].msg);
        } else {
            super(err_list['GENERIC_ERROR'].msg);
        }
        this.code = code || 'GENERIC_ERROR';
    }
    set code(value){
        this._code = value;
    }
    get code(){
        return this._code;
    }
}