__author__ = 'a.bugaev'
from selenium import webdriver
import os

local_driver = webdriver.Remote(
    command_executor='http://localhost:9999',
    desired_capabilities={'app': "C:\\Users\\" + os.environ.get("USERNAME") + r"\AppData\Roaming\ICQ\bin\icq.exe"
                          }
)


def runLoginTestUIN():
    window = local_driver.find_element_by_name('StartWindowChangeLoginType')
    window.click()

    uin_field = local_driver.find_element_by_name('StartWindowUinField')
    uin_field.send_keys('680582672')

    password_field = local_driver.find_element_by_name('StartWindowPasswordField')
    password_field.send_keys('Test1231')

    login_button = local_driver.find_element_by_name('StartWindowLoginButton')
    login_button.click()


def open_chat_and_send_message():
    click_contact = local_driver.find_element_by_name('323469493')
    click_contact.click()
    textbox = local_driver.find_element_by_name('InputTextEdit')
    textbox.send_keys('asdfasdfasdf')
    textbox.send_keys('{{}{ENTER}')

runLoginTestUIN()
open_chat_and_send_message()