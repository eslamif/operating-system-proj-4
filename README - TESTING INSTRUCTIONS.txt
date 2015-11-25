#I coudldn't get the test script working, so I followed the example in the instructions to test my program. Please copy and paste the following in the terminal. Please wait a brief moment after calling compileall.sh for the server to connect. The rest can be pasted at once.

chmod +x compileall.sh
./compileall.sh

#If you want to see the encryption and decryption daemon logs, pls cat these
cat log_otp_enc_d.txt
cat log_otp_dec_d.txt

./keygen 36 > mykey 
./run_otp_enc plaintext1 mykey 22776 > ciphertext1
cat ciphertext1

./run_otp_dec ciphertext1 mykey 26008 > plaintext1_b
cat plaintext1_b

pkill -f run_otp_enc_d
pkill -f run_otp_dec_d
