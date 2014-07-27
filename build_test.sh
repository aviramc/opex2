#!/usr/bin/env bash

gcc -g -Wall -Wunused -std=gnu99 rb_tree_test.c rb_tree.c -o rb_tree_test -lm
