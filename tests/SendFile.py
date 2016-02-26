#coding: utf-8

__author__ = 'a.bugaev'

from selenium import webdriver
import os
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
import time


local_driver = webdriver.Remote(
    command_executor='http://localhost:9999',
    desired_capabilities={'app': "C:\\Users\\" + os.environ.get("USERNAME") + r"\AppData\Roaming\ICQ\bin\icq.exe"
                          }
)


def run_login_test_by_phonenumber():
    phonenumber_field = local_driver.find_element_by_name('StartWindowPhoneNumberField')
    phonenumber_field.send_keys('79163865539')

    continue_button = local_driver.find_element_by_name('StartWindowLoginButton')
    continue_button.click()

    sms_code_field = local_driver.find_element_by_name('StartWindowSMScodeField')
    sms_code_field.send_keys('5539')

def open_chat():
    open_chat = local_driver.find_element_by_name('609929080')
    open_chat.click()


def send_file(filename):
    clip = local_driver.find_element_by_name('SendFileButton')
    clip.click()
    windows_explorer = local_driver.find_element_by_name('Рабочий стол')
    windows_explorer.click()
    windows_explorer = local_driver.find_element_by_name('Адрес: Рабочий стол')
    windows_explorer.click()
    windows_explorer.send_keys('C:\TestPictures')
    windows_explorer.submit()

    actions = ActionChains(local_driver)
    actions.double_click(local_driver.find_element_by_name(filename))
    actions.perform()

run_login_test_by_phonenumber()
open_chat()
send_file('maxresdefault.jpg')
time.sleep(5)
send_file('icqsetup.exe')





