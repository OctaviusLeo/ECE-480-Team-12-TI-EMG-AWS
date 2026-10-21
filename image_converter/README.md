1. Run PowerShell as Administrator, then execute: Set-ExecutionPolicy RemoteSigned
2. IDE's terminal via Powershell, go to image_converter folder: cd image_converter
3. Create the venv once (it is not committed): python -m venv .venv
4. Activate it: .venv\Scripts\Activate.ps1, then install the dependency: pip install -r requirements.txt
5. Have png inside same root folder.
6. Run and replace with exact png name: python png2pal4.py PVP_TIE_PIC.png PVP_TIE_PIC
7. This will generate .c and .h files that you put into project folder to use for MCU's OLED. 