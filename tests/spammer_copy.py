# coding: utf-8
import os
import socket
import urllib
import urllib.parse
import urllib.error
import urllib.request
import logging
import json
import time

if 'http_proxy' in os.environ:
    del os.environ['http_proxy']

API_HOST = 'api.icq.net'
API_KEY = 'ic18OVzN4NKoe5XO'

logging.basicConfig(
    level=logging.DEBUG,
    format='[%(asctime)s] [%(process)d] [%(processName)s] [%(levelname)s] %(message)s'
)

logger = logging.getLogger(__name__)


class OpenUrlError(Exception):
    pass


def open_url(url, timeout=5):
    try:
        return urllib.request.urlopen(url).read()
    except urllib.error.URLError as e:
        logger.exception('Failed open url "%s": %s', url, e)
        raise OpenUrlError(str(e))
    except socket.timeout as e:
        logger.error('Timeout on open url "%s": %s', url)
        raise OpenUrlError(str(e))


def api_response_success(response):
    response = response.decode('utf-8')
    response = json.loads(response)
    try:
        if (response['response']['statusCode'] == 200 and
                    response['response']['statusText'].lower() == 'ok'):
            return True
    except KeyError:
        pass
    return False


class SendMessageError(Exception):
    pass


def send_message(uin_from, name_from, uin_to, message):
    url = 'http://%s/trusted_im/send' % API_HOST
    params = {
        'k': API_KEY,
        'f': 'json',
        's': uin_to,
        'Sender': uin_from,
        'SenderAlias': name_from.encode('utf-8'),
        'message': message.encode('utf-8'),
        'SentTime': int(time.time()),
    }
    url = '%s?%s' % (url, urllib.parse.urlencode(params))
    method = 'send_message'
    logger.debug('%s: uin=%s, url=%s', method, uin_to, url)
    try:
        response = open_url(url)
        logger.debug('%s: uin=%s, response=%s', method, uin_to, response)
    except OpenUrlError as e:
        logger.error('%s failed: uin=%s: %s', method, uin_to, e)
        raise SendMessageError()
    if api_response_success(response):
        logger.info('%s success: uin=%s', method, uin_to)
    else:
        logger.error('%s failed by response: uin=%s: %s',
                     method, uin_to, response)
        raise SendMessageError()



def send_spam():
    uin_from = 693845107  # int(input())
    name_from = 'Spam user'
    to_uin = '652899761'  # int(input())
    count = 500  # int(input())
    for i in range(count):
        msg = 'test5555555 %d' % (i + 1)
        send_message(uin_from, name_from, to_uin, msg)
        print(i)
        time.sleep(0.7)



send_spam()
