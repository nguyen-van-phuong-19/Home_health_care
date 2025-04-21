import serial
import serial.tools.list_ports
import struct
import sys
from time import sleep

def find_arduino_port(baudrate=115200):
    """Tìm cổng serial có dữ liệu hợp lệ"""
    ports = list(serial.tools.list_ports.comports())
    for port in ports:
        try:
            ser = serial.Serial(port.device, baudrate=baudrate, timeout=1)
            print(f"Testing {port.device}...")
            
            # Đọc thử 4 byte và kiểm tra
            data = ser.read(4)
            if len(data) == 4:
                print(f"Connected to {port.device}")
                return ser
            ser.close()
        except (serial.SerialException, PermissionError):
            continue
    return None

def read_float_from_uart(ser):
    """Đọc và giải mã dữ liệu float từ UART"""
    buffer = bytearray()
    while True:
        try:
            # Đọc từng byte và tích lũy
            while len(buffer) < 4:
                byte = ser.read(1)
                if byte:
                    buffer += byte
            
            # Giải mã 4 byte thành float (little-endian)
            value = struct.unpack('<f', buffer[:4])[0]  # '<f' cho little-endian
            buffer = buffer[4:]  # Xóa dữ liệu đã xử lý
            
            return value
            
        except KeyboardInterrupt:
            ser.close()
            print("\nConnection closed")
            sys.exit(0)
        except Exception as e:
            print(f"Error: {str(e)}")
            return None

if __name__ == "__main__":
    # Cấu hình
    BAUDRATE = 115200  # Phải khớp với tốc độ baud của STM32
    
    # Tìm cổng
    ser = find_arduino_port(baudrate=BAUDRATE)
    if not ser:
        print("No valid UART port found!")
        sys.exit(1)
    
    # Đọc dữ liệu liên tục
    try:
        while True:
            pressure = read_float_from_uart(ser)
            if pressure is not None:
                print(f"Message: {pressure:.2f}")
    except KeyboardInterrupt:
        ser.close()
        print("Program terminated")