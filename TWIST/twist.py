#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
twist.py: Does really cool stuff

Usage:
   twist.py [options] [-q | -v] ([-n <nodeid>]... | --all)
   twist.py --config

Options:
   -i <image>, --image <image>     image that should be installed (exe file)
   -n <nodeid>, --node <nodeid>    node number(s)
   -a, --all                       use all available nodes

Other options:
   --session <file>    twist session cookie store [default: /tmp/twist_cookies]
   --config            prints empty config file
   -c=<conf>           loads config file
   -C                  loads config from ~/.config/twist.json
   -h, --help          show this help message and exit
   -q, --quiet         print less text
   -v, --verbose       print more text
   --version           show version and exit
"""

__author__ = "Mikolaj Chwalisz"
__copyright__ = "Copyright (c) 2015, Technische Universität Berlin"
__version__ = "0.1.0"
__email__ = "chwalisz@tkn.tu-berlin.de"

import logging
import requests
import getpass
import pickle
from lxml import html
from bs4 import BeautifulSoup
twist_url = "https://www.twist.tu-berlin.de:8000/"
bin_url = "http://httpbin.org/post"
log = logging.getLogger('twist')

def login(login=None, password=None, cookie_file=None):
	"""login() -> returns requests session to use in other functions"""
	if cookie_file is not None:
		try:
			with open(cookie_file) as f:
				cookies = requests.utils.cookiejar_from_dict(pickle.load(f))
				twist_session = requests.Session()
				twist_session.cookies = cookies
		except IOError:
			twist_session = requests.Session()
	else:
		twist_session = requests.Session()

	# Check session:
	check = twist_session.get(twist_url, verify=False)
	check_soup = BeautifulSoup(check.text)
	check_soup = check_soup.find(id="menu")
	for link in check_soup.find_all("a"):
		if "login" in link["href"]:
			if login is None:
				login = raw_input("TWIST login: ")
			if password is None:
				password = getpass.getpass()
			login_data = {"username": login, "password": password, "commit": "Sign in"}
			login_post = twist_session.post(twist_url + "__login__", data=login_data, verify=False)
			break
	with open(cookie_file, 'w') as f:
	    pickle.dump(requests.utils.dict_from_cookiejar(twist_session.cookies), f)
	return twist_session
# def login

def get_jobs(session):
	"""get_jobs() -> array of all scheduled jobs"""
	jobs_get = session.get(twist_url + "jobs", verify=False)
	# Process result
	jobs_soup = BeautifulSoup(jobs_get.text)
	jobs_soup = jobs_soup.find(id="primaryContent")
	headers = [header.string for header in jobs_soup.find_all("th")]
	jobs = []
	for row in jobs_soup.find_all("tr"):
		columns = row.find_all("td")
		if not columns:
			continue
		job = { headers[1]: columns[1].get_text(), # Id
				headers[2]: columns[2].get_text(), # Description
				headers[3]: columns[3].get_text(), # Owner
				headers[4]: columns[4].get_text(), # Resources
				headers[5]: columns[5].get_text(), # Start
				headers[6]: columns[6].get_text(), # End
				"active": True if "active" in columns[0].get("class") else False
				}
		jobs.append(job)
	return jobs
# def get_jobs

def get_active_jobid(session):
	jobs = get_jobs(session)
	# Search for active Telos job
	try:
		active_job = [job for job in jobs if job["active"] and "Telos" in job["Resources"]][0]
	except IndexError, e:
		return None
	return active_job["Id"]

def get_node_ids(session, job_id):
	"""get_nodeids() -> docstring"""
	current_job = session.get(twist_url + "jobs/control?job_id=" + job_id)
	job_soup = BeautifulSoup(current_job.text)
	job_soup = job_soup.find(id="controlJob-res-p5-field")
	job_soup = job_soup.find("textarea")
	node_ids = [int(i) for i in job_soup.text.split(' ')]
	return node_ids
# def get_nodeids


def install(session, job_id, filename, nodes):
	"""install() -> docstring"""
	data = {
		"__nevow_form__": "controlJob",
		"job_id": job_id,
		"ctrl.grp1.image": open(filename, 'rb'),
		"ctrl.grp1.nodes": ' '.join(map(str, nodes)),
		"ctrl.grp1.sfversion": "2",
		"ctrl.grp1.sfspeed": "115200",
		"install": "Install"
		}
	log.debug("Trying to install {0} on nodes: {1}.".format(filename, ' '.join(map(str, nodes))))
	log.info("Installing... this can take couple of minutes. Be patient")
	install = session.post(twist_url +  "jobs/control", files=data, verify=False)
	log.info(install.text)
# def install


def main(args):
	"""Run the code for twist"""
	log.debug(args)
	# Login to TWIST testbed
	session = login(cookie_file=args["--session"])
	# Get current job list
	jobid = get_active_jobid(session)
	if args["--all"]:
		nodeids = get_node_ids(session, jobid)
	else:
		nodeids = args["--node"]
	install(session, jobid, args['--image'], nodeids)
	# twist_install(None, 20, "source/build/telosb/main.exe", "229")
# def main


def parse_json_config(args):
	"""
	Takes care of the correct configure file management.

	It either prints the empty json config structure or adds the
	parameters from the given one to the existing arguments (args)
	"""
	import json
	if args['--config']:
		del args['-c']
		del args['-C']
		del args['--config']
		del args['--help']
		del args['--quiet']
		del args['--verbose']
		del args['--version']
		print(json.dumps(args, sort_keys=True, indent=4))
		exit()
	def merge_configs(args, filename):
		json_config = json.loads(open(filename).read())
		return dict((str(key), args.get(key) or json_config.get(key))
			for key in set(json_config) | set(args))
	if args['-c']:
		return merge_configs(args, args['-c'])
	elif args['-C']:
		return merge_configs(args, "~/.config/twist.json")
	else:
		return args
# def parse_json_config

if __name__ == "__main__":
	try:
		from docopt import docopt
	except:
		print("""
		Please install docopt using:
			pip install docopt==0.6.1
		For more refer to:
		https://github.com/docopt/docopt
		""")
		raise

	args = docopt(__doc__, version=__version__)
	args = parse_json_config(args)

	log_level = logging.INFO  # default
	if args['--verbose']:
		log_level = logging.DEBUG
	elif args['--quiet']:
		log_level = logging.ERROR
	logging.basicConfig(level=log_level,
		format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
	main(args)
