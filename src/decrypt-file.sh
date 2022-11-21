#!/bin/sh
gpg --pinentry-mode=loopback --passphrase=%%PASSPHRASE%% --cipher-algo=AES256 --quiet -d %%ENCRYPTED-FILE%% > %%UNENCRYPTED-FILE%%
