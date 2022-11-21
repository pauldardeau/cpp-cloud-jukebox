#!/bin/sh
gpg --pinentry-mode=loopback --passphrase=%%PASSPHRASE%% --cipher-algo=AES256 -c %%UNENCRYPTED-FILE%%
