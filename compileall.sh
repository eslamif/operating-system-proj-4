#!/bin/bash
#Frank Eslami, CS344-400, Program 4

#Compile all programs
gcc -Wall keygen.c -o keygen
gcc otp_enc_d.c -o run_otp_enc_d
gcc otp_dec_d.c -o run_otp_dec_d
gcc otp_enc.c -o run_otp_enc
gcc otp_dec.c -o run_otp_dec

./run_otp_enc_d 22776 &
./run_otp_dec_d 26008 &

