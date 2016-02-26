__author__ = 'a.bugaev'
from selenium import webdriver
import os
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

    time.sleep(3)

    sms_code_field = local_driver.find_element_by_name('StartWindowSMScodeField')
    sms_code_field.send_keys('5539')

    login_button = local_driver.find_element_by_name('StartWindowLoginButton')
    login_button.click()


run_login_test_by_phonenumber()
