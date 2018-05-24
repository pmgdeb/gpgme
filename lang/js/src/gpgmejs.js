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

import {Connection} from "./Connection"
import {GPGME_Message, createMessage} from './Message'
import {toKeyIdArray} from "./Helpers"
import { gpgme_error } from "./Errors"
import { GPGME_Keyring } from "./Keyring";

export class GpgME {
    /**
     * initializes GpgME by opening a nativeMessaging port
     * TODO: add configuration
     */
    constructor(connection){
        this.connection = connection;
    }

    set connection(conn){
        if (this._connection instanceof Connection){
            gpgme_error('CONN_ALREADY_CONNECTED');
        } else if (conn instanceof Connection){
            this._connection = conn;
        } else {
            gpgme_error('PARAM_WRONG');
        }
    }

    get connection(){
        if (this._connection){
            if (this._connection.isConnected === true){
                return this._connection;
            }
            return undefined;
        }
        return undefined;
    }

    set Keyring(keyring){
        if (keyring && keyring instanceof GPGME_Keyring){
            this._Keyring = keyring;
        }
    }

    get Keyring(){
        return this._Keyring;
    }

    /**
     * @param {String} data text/data to be encrypted as String
     * @param  {GPGME_Key|String|Array<String>|Array<GPGME_Key>} publicKeys Keys used to encrypt the message
     * @param {Boolean} wildcard (optional) If true, recipient information will not be added to the message
     */
    encrypt(data, publicKeys, base64=false, wildcard=false){

        let msg = createMessage('encrypt');
        if (msg instanceof Error){
            return Promise.reject(msg)
        }
        // TODO temporary
        msg.setParameter('armor', true);
        msg.setParameter('always-trust', true);
        if (base64 === true) {
            msg.setParameter('base64', true);
        }
        let pubkeys = toKeyIdArray(publicKeys);
        msg.setParameter('keys', pubkeys);
        putData(msg, data);
        if (wildcard === true){
            msg.setParameter('throw-keyids', true);
        };
        if (msg.isComplete === true){
            return this.connection.post(msg);
        } else {
            return Promise.reject(gpgme_error('MSG_INCOMPLETE'));
        }
    }

    /**
    * @param  {String} data TODO base64? Message with the encrypted data
    * @param {Boolean} base64 (optional) Response should stay base64
    * @returns {Promise<Object>} decrypted message:
        data:   The decrypted data.  This may be base64 encoded.
        base64: Boolean indicating whether data is base64 encoded.
        mime:   A Boolean indicating whether the data is a MIME object.
        info:   An optional object with extra information.
    * @async
    */

    decrypt(data, base64=false){
        if (data === undefined){
            return Promise.reject(gpgme_error('MSG_EMPTY'));
        }
        let msg = createMessage('decrypt');
        if (base64 === true){
            msg.expected = 'base64';
        }
        if (msg instanceof Error){
            return Promise.reject(msg);
        }
        putData(msg, data);
        return this.connection.post(msg);

    }

    sign(data, keys, mode='clearsign', base64=false) { //sender
        if (data === undefined){
            return Promise.reject(gpgme_error('MSG_EMPTY'));
        }
        let key_arr = toKeyIdArray(keys);
        if (key_arr.length === 0){
            return Promise.reject(gpgme_error('MSG_NO_KEYS'));
        }
        let msg = createMessage('sign');

        msg.setParameter('keys', key_arr);
        if (base64 === true){
            msg.setParameter('base64', true);
        }
        msg.setParameter('mode', mode);
        putData(msg, data);
        if (mode === 'detached') {
            msg.expected = 'base64';
        }
        let me = this;
        return new Promise(function(resolve,reject) {
            me.connection.post(msg).then( function(message) {
                if (mode === 'clearsign'){
                    resolve({
                        data: message.data}
                    );
                } else if (mode === 'detached') {
                    resolve({
                        data: data,
                        signature: message.data
                    });
                }
            }, function(error){
                reject(error);
            })
        });
    }

    deleteKey(key, delete_secret = false, no_confirm = false){
        return Promise.reject(gpgme_error('NOT_YET_IMPLEMENTED'));
        let msg = createMessage('deletekey');
        if (msg instanceof Error){
            return Promise.reject(msg);
        }
        let key_arr = toKeyIdArray(key);
        if (key_arr.length !== 1){
            return Promise.reject(
                gpgme_error('GENERIC_ERROR'));
            // TBD should always be ONE key?
        }
        msg.setParameter('key', key_arr[0]);
        if (delete_secret === true){
            msg.setParameter('allow_secret', true);
            // TBD
        }
        if (no_confirm === true){ //TODO: Do we want this hidden deep in the code?
            msg.setParameter('delete_force', true);
            // TBD
        }
        if (msg.isComplete === true){
            this.connection.post(msg).then(function(success){
                // TODO: it seems that there is always errors coming back:
            }, function(error){
                switch (error.msg){
                case 'ERR_NO_ERROR':
                    return Promise.resolve('okay'); //TBD
                default:
                    return Promise.reject(gpgme_error('TODO') ); //
                    // INV_VALUE,
                    // GPG_ERR_NO_PUBKEY,
                    // GPG_ERR_AMBIGUOUS_NAME,
                    // GPG_ERR_CONFLICT
                }
            });
        } else {
            return Promise.reject(gpgme_error('MSG_INCOMPLETE'));
        }
    }
}

/**
 * Sets the data of the message
 * @param {GPGME_Message} message The message where this data will be set
 * @param {*} data The data to enter
 */
function putData(message, data){
    if (!message || !message instanceof GPGME_Message ) {
        return gpgme_error('PARAM_WRONG');
    }
    if (!data){
        return gpgme_error('PARAM_WRONG');
    } else if (typeof(data) === 'string') {
        message.setParameter('data', data);
    } else if (
        typeof(data) === 'object' &&
        typeof(data.getText) === 'function'
    ){
        let txt = data.getText();
        if (typeof(txt) === 'string'){
            message.setParameter('data', txt);
        } else {
            return gpgme_error('PARAM_WRONG');
        }

    } else {
        return gpgme_error('PARAM_WRONG');
    }
}