__author__ = 'a.bugaev'

import os
from selenium import webdriver
import time

local_driver = webdriver.Remote(
    command_executor='http://localhost:9999',
    desired_capabilities={'app': "C:\\Users\\" + os.environ.get("USERNAME") + r"\AppData\Roaming\ICQ\bin\icq.exe"
                          }
)


def enter_wrong_pass():
    window = local_driver.find_element_by_name('StartWindowChangeLoginType')
    window.click()

    uin_field = local_driver.find_element_by_name('StartWindowUinField')
    uin_field.send_keys('680582672')

    password_field = local_driver.find_element_by_name('StartWindowPasswordField')
    password_field.send_keys('Test1230')

    login_button = local_driver.find_element_by_name('StartWindowLoginButton')
    login_button.click()


def enter_true_pass():
    password_field = local_driver.find_element_by_name('StartWindowPasswordField')
    password_field.send_keys('Test1231')

    login_button = local_driver.find_element_by_name('StartWindowLoginButton')
    login_button.click()



enter_wrong_pass()
time.sleep(3)
enter_true_pass()
