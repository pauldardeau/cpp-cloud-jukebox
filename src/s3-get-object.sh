#!/bin/sh
export S3_ACCESS_KEY_ID="%%S3_ACCESS_KEY%%"
export S3_SECRET_ACCESS_KEY="%%S3_SECRET_KEY%%"
export S3_HOSTNAME="%%S3_HOST%%"
export BUCKET_NAME="%%BUCKET_NAME%%"
export OBJECT_NAME="%%OBJECT_NAME%%"
s3 get $BUCKET_NAME/$OBJECT_NAME filename=song-play/$OBJECT_NAME
