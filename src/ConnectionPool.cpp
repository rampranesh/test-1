/*
 * Copyright (C) 1994-2017 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * PBS Pro is free software. You can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * The PBS Pro software is licensed under the terms of the GNU Affero General
 * Public License agreement ("AGPL"), except where a separate commercial license
 * agreement for PBS Pro version 14 or later has been executed in writing with Altair.
 *
 * Altair’s dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of PBS Pro and distribute
 * them - whether embedded or bundled with other software - under a commercial
 * license agreement.
 *
 * Use of Altair’s trademarks, including but not limited to "PBS™",
 * "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
 * trademark licensing policies.
 *
 */

#include "ConnectionPool.h"

extern "C" {
#include <pbs_ifl.h>
}

ConnPool* ConnPool::_instance = 0;
pthread_mutex_t ConnPool::_instMutex = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief
 *      getConnectionFromPool() - creates a new connection to DRMS(PBSPro) if there is no existing, unused connection,
 *					else returns the existing one.
 *
 * @param[in]   ServerName_    	- Name of server which we need to connect
 *
 * @par Side-Effects: none
 *
 * @par    MT-safe
 *
 * @return   	>0 - If connection successful and pool is not full
 *		-1 - If pool is full or connection was not successful
 *
 */

int ConnPool::getConnectionFromPool(char *serverName_) {
	pthread_mutex_lock(&_instMutex);
	if(_connPool.empty()) {
		Connection connection_;
		connection_._fd = pbs_connect(serverName_); //TODO: replace pbs_connect with DRMSystem's wrapped method 
		if(connection_._fd < 0) {
			pthread_mutex_unlock(&_instMutex);
			return -1;
		}
		connection_._serverName.assign(serverName_);
		connection_._fdInUse = true;
		_connPool.push_back(connection_);
		pthread_mutex_unlock(&_instMutex);
		return connection_._fd;
	} else if (_connPool.size() < MAX_CONNS) {
		string DRMServer(serverName_);
		for(std::list<Connection>::iterator it=_connPool.begin(); it != _connPool.end(); ++it) {
			if(DRMServer == (*it)._serverName) {
				if((*it)._fdInUse == false) {
					(*it)._fdInUse = true;
					pthread_mutex_unlock(&_instMutex);
					return (*it)._fd;
				}
			}
		}
		Connection connection_;
		connection_._fd = pbs_connect(serverName_); //TODO: replace pbs_connect with DRMSystem's wrapped method
		connection_._serverName.assign(serverName_);
		connection_._fdInUse = true;
		_connPool.push_back(connection_);
		pthread_mutex_unlock(&_instMutex);
		return connection_._fd;
	} else {
		string DRMServer(serverName_);
		for(std::list<Connection>::iterator it=_connPool.begin(); it != _connPool.end(); ++it) {
			if(DRMServer == (*it)._serverName) {
				if((*it)._fdInUse == false) {
					(*it)._fdInUse = true;
					pthread_mutex_unlock(&_instMutex);
					return (*it)._fd;
				}
			}
		}
		pthread_mutex_unlock(&_instMutex);
		return -1;
	}
}


/**
 * @brief
 *      returnConnectionToPool() - returns the fd to pool
 *
 * @param[in]   fd_     -fd which needs to be returned
 *
 * @par Side-Effects: none
 *
 * @par    MT-safe
 *
 * @return     	void
 *
 */

void ConnPool::returnConnectionToPool(int fd_) {
	pthread_mutex_lock(&_instMutex);
	for(std::list<Connection>::iterator it=_connPool.begin(); it != _connPool.end(); ++it) {
		if(fd_ == (*it)._fd) {
			(*it)._fdInUse = false;
			break;
		}
	}
	pthread_mutex_unlock(&_instMutex);
}

/**
 * @brief
 *      reestablishConnection() - creates new connection and returns the new fd.
 *
 * @param[in]   fd_     - fd which we need to re-connect
 *
 * @par Side-Effects: none
 *
 * @par    MT-safe
 *
 * @return      void
 *
 */

int ConnPool::reestablishConnection(int fd_) {
	std::string DRMServer;
	Connection connection_;
	bool foundFd = false;
	pthread_mutex_lock(&_instMutex);
	for(std::list<Connection>::iterator it=_connPool.begin(); it != _connPool.end(); ++it) {
		if(fd_ == (*it)._fd) {
			DRMServer.assign((*it)._serverName);
			_connPool.erase(it);
			foundFd = true;
			break;
		}
	}
	if(foundFd != true) {
		pthread_mutex_unlock(&_instMutex);
		return -1;
	}
	connection_._fd = pbs_connect((char *)DRMServer.c_str()); //TODO: replace pbs_connect with DRMSystem's wrapped method
	if(connection_._fd < 0) {
		pthread_mutex_unlock(&_instMutex);
		return -1;
	}
	connection_._serverName.assign(DRMServer);
	connection_._fdInUse = true;
	_connPool.push_back(connection_);
	pthread_mutex_unlock(&_instMutex);
	return connection_._fd;
}
