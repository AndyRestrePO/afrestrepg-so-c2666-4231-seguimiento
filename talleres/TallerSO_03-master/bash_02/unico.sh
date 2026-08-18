#!/bin/bash

cut -d '|' -f "$2" "$1" | sort | uniq
