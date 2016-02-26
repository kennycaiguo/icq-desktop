__author__ = 'a.bugaev'
from selenium import webdriver
from selenium.webdriver.common.keys import Keys
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

    sms_code_field = local_driver.find_element_by_name('StartWindowSMScodeField')
    sms_code_field.send_keys('5539')

def open_chat_and_send_message():
    print('searching contact...')
    time.sleep(1)
    click_contact = local_driver.find_element_by_name('609929080')
    click_contact.click()
    print('contact has been found')
    textbox = local_driver.find_element_by_name('InputTextEdit')
    textbox.send_keys('asdfasdfasdf')
    time.sleep(1)
    send_message = local_driver.find_element_by_name('SendMessageButton')
    send_message.click()
   # textbox.send_keys('{{}{ENTER}')


run_login_test_by_phonenumber()
open_chat_and_send_message()
