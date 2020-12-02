import secrets

from tinkerforge.ip_connection import base58encode, base58decode, BASE58

wifi_passphrase = [''.join(secrets.choice(BASE58) for i in range(4)) for j in range(4)]
# TODO: get uid from server when merging this script with flash-test
uid = 'ESP'

print("UID: " + uid)
print("Passphrase: {}-{}-{}-{}".format(*wifi_passphrase))

uid_bytes = base58decode(uid).to_bytes(4, byteorder='little')
passphrase_bytes_list = [base58decode(chunk).to_bytes(3, byteorder='little') for chunk in wifi_passphrase]

print(["{:02x}".format(i) for i in uid_bytes])
print([''.join("{:02x}".format(i) for i in chunk) for chunk in passphrase_bytes_list])

#56-95: 5 byte
#160-183: 3 byte
#192-255: 8 byte
# = 16 byte

# 4 byte (uid) + 3 byte * 4 (wifi_passphrase) = 16 byte
binary = bytearray(32)
binary[7:10] = passphrase_bytes_list[0]
binary[10:12] = passphrase_bytes_list[1][0:2]
binary[20] = passphrase_bytes_list[1][2]
binary[21:23] = passphrase_bytes_list[2][0:2]
binary[24] = passphrase_bytes_list[2][2]
binary[25:28] = passphrase_bytes_list[3]
binary[28:32] = uid_bytes

with open("efuse.bin", "wb") as f:
    f.write(binary)
