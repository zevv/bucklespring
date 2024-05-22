import ctypes
import os

lib_path = os.path.abspath("/home/hounaar/Desktop/parsa/bucklespring/libbuckle.so")
buckle_lib = ctypes.CDLL(lib_path)

buckle_lib.main.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]

def start_bucklespring():
    argc = ctypes.c_int(1)
    argv = (ctypes.c_char_p * 1)(b"./buckle")
    
    buckle_lib.main(argc, argv)

if __name__ == "__main__":
    start_bucklespring()
