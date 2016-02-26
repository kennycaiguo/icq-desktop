__author__ = 'a.bugaev'

import re
import requests
import urllib
from urllib import request
import time

def get_sessions(login = input()):

    url = 'https://stager.orange.icq.com/icq-tools/sapi-info'
    params = {
        'uin': login,
        'do': 'get',
        'filter': 'Session ID',
        'command': 'ud'
    }
    resp = requests.get(url, params=params, auth=('a.bugaev', 'PLlZEfb$E8tQ'))

    sessions = []
    for line in resp.text.split('\n'):
        match = re.search('Session ID.+: (\d+\.\d+\.\d+:\d+)', line)
        if match is not None:
            sessions.append(match.group(1))

    for session_id in sessions:
        print(session_id)
    return sessions



for session_id in get_sessions():
    session_url = 'http://api.icq.net/aim/endSession?aimsid={}&f=json&inactiveToken=true'.format(session_id)
    urllib.request.urlopen(session_url)

time.sleep(45)
print('New sessions')

get_sessions()
#get_url = urllib.request.urlopen('https://stager.orange.icq.com/icq-tools/sapi-info?uin=652899761&do=get&command=ud&filter=Session ID')