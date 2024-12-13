import os
import sys
import shutil
import ctypes
from ctypes import wintypes

def find_pico_mount():
    """
    Windows環境でRaspberry Pi Pico (RPI-RP2) が接続されているマウントポイントを探す
    """
    drive_list = get_drive_list()
    for drive in drive_list:
        if check_volume_label(drive, "RPI-RP2"):
            return drive
    return None

def get_drive_list():
    """
    現在の接続ドライブを取得
    """
    drives = []
    bitmask = ctypes.windll.kernel32.GetLogicalDrives()
    for letter in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        if bitmask & 1:
            drives.append(f"{letter}:\\")
        bitmask >>= 1
    return drives

def check_volume_label(drive, label):
    """
    ドライブのボリュームラベルを確認
    """
    buffer = ctypes.create_unicode_buffer(256)
    try:
        result = ctypes.windll.kernel32.GetVolumeInformationW(
            wintypes.LPWSTR(drive),
            buffer,
            ctypes.sizeof(buffer),
            None, None, None, None, 0
        )
        return result and buffer.value == label
    except Exception:
        return False

def copy_to_pico(uf2_file, pico_mount):
    """
    UF2ファイルをPicoのマウントポイントにコピー
    """
    try:
        print(f"UF2ファイルをコピー中: {uf2_file} -> {pico_mount}")
        shutil.copy(uf2_file, pico_mount)
        print("コピーが完了しました。")
    except Exception as e:
        print(f"コピー中にエラーが発生しました: {e}")

def main():
    if len(sys.argv) < 2:
        print("使い方: python upload_to_pico.py <UF2ファイルパス>")
        sys.exit(1)

    uf2_file = sys.argv[1]

    if not os.path.isfile(uf2_file):
        print(f"UF2ファイルが見つかりません: {uf2_file}")
        sys.exit(1)

    pico_mount = find_pico_mount()
    if not pico_mount:
        print("Raspberry Pi Pico Wがマスストレージモードで接続されていません。")
        sys.exit(1)

    copy_to_pico(uf2_file, pico_mount)

if __name__ == "__main__":
    main()
