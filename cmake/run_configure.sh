#!/bin/bash

shift
shift
shift
shift

CONFIG_PATH=$1
shift
$CONFIG_PATH $@
