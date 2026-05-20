#!/usr/bin/python3

import os

import requests
from bs4 import BeautifulSoup

URL = 'https://mentors.debian.net/'

r = requests.get(URL)
soup = BeautifulSoup(r.content, "html.parser")

for a in soup.find_all('a'):
    href = a['href']
    if href.startswith('/package/'):
        package = href.split('/')[2]
        if os.path.exists('rules/%s' % package):
            print(package)
