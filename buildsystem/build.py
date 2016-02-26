from __future__ import print_function
if __name__ != '__main__':
	sys.exit(0)

import os, sys, shutil
import fileinput
import distutils.dir_util
import requests
import zipfile
from os.path import basename

def sign_file(file_name):
	temp_file_name = file_name + ".tmp"
	os.system("curl.exe -F file=@" + file_name + " http://sign.corp.mail.ru/sign_amigo_distribution -o " + temp_file_name)
	shutil.move(temp_file_name, file_name)

def get_str_version_number(build_number):	
	ICQ_VERSION_MAJOR = '10'
	ICQ_VERSION_MINOR = '0'
	return ".".join((ICQ_VERSION_MAJOR, ICQ_VERSION_MINOR, build_number))
	
def update_version(build_number):
	prod_version_str_macro = "#define VERSION_INFO_STR"
	file_version_str_macro = "#define VER_FILEVERSION_STR"
	file_version_macro = "#define VER_FILEVERSION"

	STR_VERSION = get_str_version_number(build_number)
	VERSION = STR_VERSION.replace('.', ',')

	for filename in ("../common.shared/version_info_constants.h", ):
		for line in fileinput.input(filename, inplace=True):
			if prod_version_str_macro in line:
				line = prod_version_str_macro + ' "'+STR_VERSION+'"\n'
			elif file_version_str_macro in line:
				line = file_version_str_macro + ' "'+STR_VERSION+'"\n'
			elif file_version_macro in line:
				line = file_version_macro + ' '+VERSION+'\n'
			print(line, end='')
	
app_id = '517065520bb44e84bf2912da3b74b61f'
upload_only_token = '05f3986c5260475f888b1611bf469c11'
	
def zip_installer():
	setup = r'../bin/Release/installer/icqsetup.exe'
	pdb_installer = r'../bin/Release/installer/icqsetup.pdb'
	pdb_icq = r'../bin/Release/ICQ.pdb'
	pdb_corelib = r'../bin/Release/corelib.pdb'
	archive = r'../bin/Release/installer/icqsetup.zip'
	zf = zipfile.ZipFile(archive, mode='w')
	try:
		zf.write(setup, basename(setup))
		zf.write(pdb_installer, basename(pdb_installer))
		zf.write(pdb_icq, basename(pdb_icq))
		zf.write(pdb_corelib, basename(pdb_corelib))
	except:
		return
	finally:
		zf.close
		return archive
	
def upload_install_to_hockey_app(version_id):
	url = 'https://rink.hockeyapp.net/api/2/apps/' + app_id + '/app_versions/' + str(version_id)
	archive = zip_installer()
	resp = requests.put(url, headers={'X-HockeyAppToken': upload_only_token}, files={"ipa": open(archive, 'rb')})
	print(resp.status_code)
	
def publish_version_to_hockey_app(build_number):
	url = 'https://rink.hockeyapp.net/api/2/apps/' + app_id + '/app_versions/new'
	build_number = os.getenv('BUILD_NUMBER', '2000')
	resp = requests.post(url, data={'bundle_version': get_str_version_number(build_number)}, headers={'X-HockeyAppToken': upload_only_token})
	print("Publishing to hockeyapp:")
	print(resp.status_code, resp.reason)
	data = resp.json()
	version_id = data['id']
	upload_install_to_hockey_app(version_id)

build_path = os.getenv('MSBUILD', 'C:/Program Files (x86)/MSBuild/14.0/Bin/')
build_path = '"' + build_path + '/MSBuild.exe"'

build_number = os.getenv('BUILD_NUMBER', '1999')
print(build_path)
update_version(build_number)

if os.system(build_path + " ../icq.sln /t:Rebuild /p:Configuration=Release;Platform=Win32") != 0:
	sys.exit(1)

sign_file("../bin/Release/icq.exe")
sign_file("../bin/Release/corelib.dll")
sign_file("../bin/Release/libvoip_x86.dll")

if os.system(build_path + " ../installer/installer.sln /t:Rebuild /p:Configuration=Release;Platform=Win32") != 0:
	sys.exit(1)

sign_file("../bin/Release/installer/icqsetup.exe")

upload_folder = os.path.join(os.environ["STORAGE"], build_number).replace('\\', '/')

if not os.path.exists(upload_folder):
    os.mkdir(upload_folder)
	
symbols = os.path.join(upload_folder, "symbols").replace('\\', '/')
	
upload_folder = os.path.join(upload_folder, "Release").replace('\\', '/')

if not os.path.exists(upload_folder):
    os.mkdir(upload_folder)

distutils.dir_util.copy_tree("../bin/Release", upload_folder);

if not os.path.exists(symbols):
    os.mkdir(symbols)

publish_version_to_hockey_app(build_number)
shutil.copyfile(os.path.join(upload_folder, "corelib.exp"), os.path.join(symbols, "corelib.exp"));
shutil.copyfile(os.path.join(upload_folder, "corelib.lib"), os.path.join(symbols, "corelib.lib"));
shutil.copyfile(os.path.join(upload_folder, "corelib.pdb"), os.path.join(symbols, "corelib.pdb"));
shutil.copyfile(os.path.join(upload_folder, "coretest.pdb"), os.path.join(symbols, "coretest.pdb"));
shutil.copyfile(os.path.join(upload_folder, "icq.exp"), os.path.join(symbols, "icq.exp"));
shutil.copyfile(os.path.join(upload_folder, "icq.lib"), os.path.join(symbols, "icq.lib"));
shutil.copyfile(os.path.join(upload_folder, "icq.pdb"), os.path.join(symbols, "icq.pdb"));
shutil.copyfile(os.path.join(upload_folder, "libvoip_x86.lib"), os.path.join(symbols, "libvoip_x86.lib"));

sys.exit(0)