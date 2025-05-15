
import serial
import time
import argparse

def check_l80r(port, baud=9600, read_timeout=1.0, pmtk_timeout=0.5):
    """
    Kiểm tra module L80-R có hoạt động hay không bằng cách:
    1. Đọc các dòng NMEA trong khoảng read_timeout.
    2. Gửi lệnh PMTK605*31 và chờ phản hồi trong pmtk_timeout.
    """
    try:
        ser = serial.Serial(port, baud, timeout=read_timeout)
    except serial.SerialException as e:
        print(f"[ERROR] Không thể mở cổng {port}: {e}")
        return False

    ser.reset_input_buffer()

    # 1. Đọc NMEA
    print(f"Đang đọc NMEA từ {port} trong {read_timeout} giây...")
    start = time.time()
    nmea_alive = False
    while time.time() - start < read_timeout:
        line = ser.readline().decode('ascii', errors='ignore').strip()
        if line.startswith('$GP') or line.startswith('$GN'):
            print(f"[NMEA] {line}")
            nmea_alive = True
            break

    # 2. Gửi lệnh PMTK và chờ phản hồi
    pmtk_cmd = b'$PMTK605*31\r\n'
    print("Gửi lệnh PMTK để lấy firmware version...")
    ser.write(pmtk_cmd)
    time.sleep(pmtk_timeout)
    resp = ser.read(128).decode('ascii', errors='ignore').strip()
    pmtk_alive = False
    if resp:
        print(f"[PMTK RESP] {resp}")
        if '$PMTK705' in resp:
            pmtk_alive = True

    ser.close()

    # Đánh giá kết quả
    if nmea_alive or pmtk_alive:
        print("✅ Module L80-R đang hoạt động.")
        return True
    else:
        print("❌ Không nhận được dữ liệu từ Module L80-R.")
        return False

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Kiểm tra trạng thái hoạt động của module Quectel L80-R qua UART")
    parser.add_argument("port", help="Tên cổng serial (ví dụ: COM3 trên Windows hoặc /dev/ttyUSB0 trên Linux)")
    parser.add_argument("--baud", type=int, default=9600, help="Tốc độ baud (mặc định 9600)")
    parser.add_argument("--read-timeout", type=float, default=1.0, help="Thời gian đọc NMEA (giây)")
    parser.add_argument("--pmtk-timeout", type=float, default=0.5, help="Thời gian chờ phản hồi PMTK (giây)")
    args = parser.parse_args()

    check_l80r(args.port, baud=args.baud, read_timeout=args.read_timeout, pmtk_timeout=args.pmtk_timeout)
