#!/bin/sh
export S3_ACCESS_KEY_ID="%%S3_ACCESS_KEY%%"
export S3_SECRET_ACCESS_KEY="%%S3_SECRET_KEY%%"
export S3_HOSTNAME="%%S3_HOST%%"
s3 list | cut -f 1 -d " "
