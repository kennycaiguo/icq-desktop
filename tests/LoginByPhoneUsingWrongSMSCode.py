__author__ = 'a.bugaev'

import os
from selenium import webdriver
import time



local_driver = webdriver.Remote(
    command_executor='http://localhost:9999',
    desired_capabilities={'app': "C:\\Users\\" + os.environ.get("USERNAME") + r"\AppData\Roaming\ICQ\bin\icq.exe"
                          }
)


def enter_wrong_code():
    phonenumber_field = local_driver.find_element_by_name('StartWindowPhoneNumberField')
    phonenumber_field.send_keys('79163865539')

    continue_button = local_driver.find_element_by_name('StartWindowLoginButton')
    continue_button.click()

    time.sleep(3)

    sms_code_field = local_driver.find_element_by_name('StartWindowSMScodeField')
    sms_code_field.send_keys('5538')

def enter_true_code():
    sms_code_field = local_driver.find_element_by_name('StartWindowSMScodeField')
    sms_code_field.send_keys('5539')

enter_wrong_code()
time.sleep(3)
enter_true_code()
