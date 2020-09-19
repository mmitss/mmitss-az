from PyInstaller.utils.hooks import collect_data_files
datas = collect_data_files('flask_bootstrap')
hiddenimports=['pkg_resources.py2_warn']
