# import sys
# import os 

# try:
#     python_base_dir = sys.base_prefix 
#     project_root = os.getcwd()
#     lib_dir = os.path.join(project_root, "Lib")
    
#     VCPKG_BIN = os.getenv("VCPKG_BIN")
#     if not VCPKG_BIN:
#         raise EnvironmentError("환경변수 VCPKG_BIN을 설정해주세요.")

#     os.add_dll_directory(python_base_dir) 
    
#     if os.path.exists(lib_dir):
#         os.add_dll_directory(lib_dir)
#     else:
#         print(f"⚠️ DLL 폴더 {lib_dir}가 존재하지 않습니다. (필요시 만들어주세요)")
        
#     if os.path.exists(VCPKG_BIN):
#         os.add_dll_directory(VCPKG_BIN)
#     else:
#         print(f"⚠️ [중요] vcpkg DLL 폴더를 찾을 수 없습니다: {VCPKG_BIN}")

# except AttributeError:
#     print("Warning: os.add_dll_directory requires Python 3.8 or later.")
# except FileNotFoundError:
#     print(f"Warning: Python base directory not found: {python_base_dir}")

# try:
#     import algorithm_module
#     print("성공: C++ 모듈 'algorithm_module'을 임포트했습니다.")
# except ImportError as e:
#     print(f"오류: C++ 모듈 'algorithm_module' 임포트 실패!")
#     print(f"  > {e}")
#     print(f"  > 현재 폴더: {os.getcwd()}")
#     print(f"  > sys.path:")
#     for path in sys.path:
#         print(f"    - {path}")
#     sys.exit(1)

# from PySide6.QtWidgets import QApplication
# from src.ui import MainWindow

# cpp_router = algorithm_module.ShipRouter()
# script_dir = os.path.dirname(__file__) if "__file__" in locals() else "."
# gebco_path = os.path.join(script_dir, "data", "GEBCO_2024_sub_ice_topo.nc")
# gshhs_path = os.path.join(script_dir, "data", "GSHHS_i_L1.shp")

# success = cpp_router.initialize(
#     gebco_path=gebco_path,
#     gshhs_path=gshhs_path
# )

# if __name__ == "__main__":
#     app = QApplication(sys.argv)
    
#     w = MainWindow(router=cpp_router) 
#     w.show()
#     sys.exit(app.exec())
