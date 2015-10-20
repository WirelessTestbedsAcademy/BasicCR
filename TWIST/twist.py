#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
twist.py: Command line interface to TWIST testbed

Usage:
   twist.py [options] [-q | -v] ([-n <nodeid>]... | [--floor <floor>]... | --all )
   twist.py --config

Options (at least one of them is required):
   -i <image>, --image <image>     image that should be installed (exe file)
   -s, --ssh                       setup ssh tunnel to given nodes
   -p <pass>                       ssh password

Node options:
   -n <nodeid>, --node <nodeid>    node number(s)
   -a, --all                       use all available nodes
   -f <floor>, --floor <floor>     use nodes from floor <floor>

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

from sh import ssh
twist_url = "https://www.twist.tu-berlin.de:8000/"
log = logging.getLogger('twist')


floor_nodes = {
        "2": [ 195, 191, 223, 228, 213, 216, 192, 196, 229, 215,
               222, 208, 220, 221, 212, 203, 194, 225, 209, 207,
               224, 206, 205, 211, 230, 204, 240, 241],
        "3": [ 101, 100, 103, 102, 86, 85, 87, 99, 83, 84, 80,
               79, 81, 82, 189, 190, 193, 188, 198, 187, 199,
               202, 200, 197, 218, 250, 251, 252, 262, 249, 186,
               185, 231, 214],
        "4": [ 153, 154, 151, 152, 12, 13, 10, 11, 149, 150, 147,
               148, 88, 15, 89, 90, 145, 146, 144, 143, 92, 91, 93,
               94, 139, 140, 141, 142, 96, 95, 97, 137, 138, 272]
        }

def login(login=None, password=None, cookie_file=None):
	"""Login to the TWIST testbed

	Sets up the session to the TWIST testbed. First it will try to restore it from
	the ``cookie_file`` (and store it there afterwards), if not successful it will
	look at provided arguments (if not None) will try to login. If no arguments
	are provided it will ask for credentials through ``stdin``.


	Args:
		login (Optional[str]): User name.
			Defaults to None.
		password (Optional[str]): User password.
			Defaults to None.
		cookie_file(Optional[str]): Cookie file name with stored testbed session.
			Defaults to None.

	Returns:
		requests.Session: An authenticated session to the TWIST testbed.
			It can be used by all other functions
	"""
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
	check_soup = BeautifulSoup(check.text,"lxml")
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
	"""Returns a list of IDs of currently scheduled jobs.

	Args:
		session: Authenticated ``requests.Session`` with TWIST web interface.

	Returns:
		list: Currently scheduled job IDs. Empty if no jobs are scheduled.
	"""
	jobs_get = session.get(twist_url + "jobs", verify=False)
	# Process result
	jobs_soup = BeautifulSoup(jobs_get.text, "lxml")
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
	"""Returns ID of currently active job.

	Args:
		session: Authenticated ``requests.Session`` with TWIST web interface.

	Returns:
		int: Currently active job IDs. ``None`` if no jobs are scheduled.
	"""
	jobs = get_jobs(session)
	# Search for active Telos job
	try:
		active_job = [job for job in jobs if job["active"] and "Telos" in job["Resources"]][0]
	except IndexError, e:
		return None
	return active_job["Id"]

def get_node_ids(session, job_id):
	"""Returns list of available telosb nodes in the currently active job.

	Args:
		session: Authenticated ``requests.Session`` with TWIST web interface.

	Returns:
		list: List of available telosb nodes.
	"""
	current_job = session.get(twist_url + "jobs/control?job_id=" + job_id)
	job_soup = BeautifulSoup(current_job.text, "lxml")
	job_soup = job_soup.find(id="controlJob-res-p5-field")
	job_soup = job_soup.find("textarea")
	node_ids = [int(i) for i in job_soup.text.split(' ')]
	return node_ids
# def get_nodeids


def install(session, job_id, filename, nodes):
	"""Installs given image on the nodes in the TWIST testbed.

	Args:
		session: Authenticated ``requests.Session`` with TWIST web interface.
		job_id (int): ID of currently active job.
		filename (str): File name of the image that should be installed on the nodes
		nodes (list): List of node IDs on which the image should be installed

	"""
	allowed_nodes = get_node_ids(session, job_id)
	nodes = list(set(allowed_nodes) & set(nodes))
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

__ssh_pass__ = None
__ssh_out_aggredate__ = ""
def __ssh_interact__(char, stdin):
	""" Internal function to interact with ssh session """
	global __ssh_out_aggredate__
	global __ssh_pass__
	__ssh_out_aggredate__ += char
	if __ssh_out_aggredate__.endswith("password: "):
		if __ssh_pass__ is None:
			print "Provide ssh password: "
			__ssh_pass__ = getpass.getpass()
		stdin.put("{}\n".format(__ssh_pass__))

def ssh_tunnel(nodes, passwd=None):
	""" Sets up an SSH tunnel to a given set of nodes

	Args:
		nodes (list): List of node IDs on which the image should be installed

	Note: It will prompt for ssh password.

		DO NOT EXPECT THIS FUNCTION TO EXIT.
	"""
	if passwd is not None:
		global __ssh_pass__
		__ssh_pass__ = passwd
	args = ["-nNxT4"]
	for node in nodes:
		args.append("-L")
		args.append("9{id:03d}:localhost:9{id:03d}".format(id=node))
	args.append("twistextern@www.twist.tu-berlin.de")
	ssh(args, _out=__ssh_interact__, _out_bufsize=0, _tty_in=True)
	ssh.wait()

def main(args):
	"""Run the code for twist"""
	log.debug(args)
	# Login to TWIST testbed
	session = login(cookie_file=args["--session"])
	# Get current job list
	jobid = get_active_jobid(session)
	if args["--all"]:
		nodeids = get_node_ids(session, jobid)
	elif args["--node"]:
		nodeids = [int(i) for i in args["--node"]]
	elif args["--floor"]:
		nodeids = []
		for f in args["--floor"]:
			nodeids.extend(floor_nodes[f])
	if args["--image"]:
		install(session, jobid, args['--image'], nodeids)
	if args["--ssh"]:
		ssh_tunnel(nodeids,args["-p"])
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
