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
#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H
#include <iostream>
#include <list>
#include <string>

using namespace std;

#define MAX_CONNS 20

/**
  *  @brief Class that maintains File descriptor, Port, Server name and value to indicate its status.  
  */  
class Connection {
	public:
		int _fd;
		int _port;
		string _serverName;
		bool _fdInUse;
		Connection () {
			_fd = -1;
			_port = 0;
			_fdInUse = false;
		}
};

/**
  *  @brief Class that maintains pool of connections to DRMS.  
  */  
class ConnPool {
	private:
		static pthread_mutex_t _instMutex;
		static ConnPool* _instance;
		/**
		 * @brief
		 *      ConnPool() - constructor for ConnPool
		 *
		 * @par Side-Effects: none
		 *
		 * @par    MT-safe
		 *
		 */
		ConnPool() {}
		list<Connection> _connPool;
	public:
		/**
		 * @brief
		 *	getInstance() - returns singleton Instance of ConnPool
		 *
		 * @par Side-Effects: none
		 *
		 * @par    MT-safe
		 *
		 * @return    pointer to ConnPool object
		 *
		 */
		static ConnPool* getInstance() {
			pthread_mutex_lock(&_instMutex);
			if (_instance == 0)
			{
				_instance = new ConnPool;
			}
			pthread_mutex_unlock(&_instMutex);
			return _instance;
		}
		int getConnectionFromPool(char *serverName_);
		void returnConnectionToPool(int fd_);
		int reestablishConnection(int fd_);
};

#endif
