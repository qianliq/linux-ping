import struct

def ip_checksum(data):
    """Calculate the IP header checksum."""
    if len(data) % 2:
        data += b'\x00'
    checksum = 0
    for i in range(0, len(data), 2):
        word = (data[i] << 8) + data[i+1]
        checksum += word
        checksum = (checksum & 0xffff) + (checksum >> 16)
    return ~checksum & 0xffff

def parse_ip_header(packet):
    """Parse the IP header and return a dict of fields."""
    fields = struct.unpack('!BBHHHBBH4s4s', packet[:20])
    header = {
        'version': fields[0] >> 4,
        'ihl': fields[0] & 0xF,
        'tos': fields[1],
        'total_length': fields[2],
        'id': fields[3],
        'flags_offset': fields[4],
        'ttl': fields[5],
        'protocol': fields[6],
        'checksum': fields[7],
        'src_ip': '.'.join(map(str, fields[8])),
        'dst_ip': '.'.join(map(str, fields[9])),
    }
    return header

def check_ip_checksum(packet_hex):
    """Check the checksum of an IP packet (hex string)."""
    packet = bytes.fromhex(packet_hex)
    header = parse_ip_header(packet)
    header_bytes = packet[:header['ihl']*4]
    calc_checksum = ip_checksum(header_bytes)
    print("IP Header Fields:")
    for k, v in header.items():
        print(f"  {k}: {v}")
    print(f"\nOriginal checksum in packet: 0x{header['checksum']:04x}")
    print(f"Calculated checksum:         0x{calc_checksum:04x}")
    if calc_checksum == 0:
        print("Checksum is CORRECT.")
    else:
        print("Checksum is INCORRECT.")

if __name__ == "__main__":
    # 示例：输入一个IP包的16进制字符串
    # 例如: '4500003c1c4640004006b1e6c0a80001c0a800c7'
    packet_hex = input("Enter IP packet (hex): ").strip()
    check_ip_checksum(packet_hex)