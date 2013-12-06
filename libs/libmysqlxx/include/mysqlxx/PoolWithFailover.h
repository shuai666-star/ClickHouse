#pragma once

#include "Pool.h"


#define MYSQLXX_POOL_WITH_FAILOVER_DEFAULT_START_CONNECTIONS	1
#define MYSQLXX_POOL_WITH_FAILOVER_DEFAULT_MAX_CONNECTIONS		16
#define MYSQLXX_POOL_WITH_FAILOVER_DEFAULT_MAX_TRIES			3


namespace mysqlxx
{
	/** Пул соединений с MySQL.
	  * Знает о наборе реплик с приоритетами.
	  * Пробует соединяться с репликами в порядке приоритета. При равном приоритете предпочитается реплика, к которой дольше всего не было попытки подключения.
	  *
	  * Использование аналогично mysqlxx::Pool. В конфиге задание сервера может выглядеть так же, как для Pool:
	  * <mysql_metrica>
	  * 	<host>mtstat01c*</host>
	  * 	<port>3306</port>
	  * 	<user>metrica</user>
	  * 	<password></password>
	  * 	<db>Metrica</db>
	  * </mysql_metrica>
	  *
	  * или так:
	  *
	  * <mysql_metrica>
	  * 	<replica>
	  * 		<host>mtstat01c</host>
	  * 		<port>3306</port>
	  * 		<user>metrica</user>
	  * 		<password></password>
	  * 		<db>Metrica</db>
	  * 		<priority>0</priority>
	  * 	</replica>
	  * 	<replica>
	  * 		<host>mtstat01d</host>
	  * 		<port>3306</port>
	  * 		<user>metrica</user>
	  * 		<password></password>
	  * 		<db>Metrica</db>
	  * 		<priority>1</priority>
	  * 	</replica>
	  * </mysql_metrica>
	  *
	  * или так:
	  *
	  *  <mysql_metrica>
	  *		<port>3306</port>
	  * 	<user>metrica</user>
	  * 	<password></password>
	  * 	<db>Metrica</db>
	  *		<replica>
	  * 		<host>mtstat01c</host>
	  * 		<priority>0</priority>
	  * 	</replica>
	  * 	<replica>
	  * 		<host>mtstat01d</host>
	  * 		<priority>1</priority>
	  * 	</replica>
	  * </mysql_metrica>
	  */
	class PoolWithFailover
	{
	private:
		typedef Poco::SharedPtr<Pool> PoolPtr;

		struct Replica
		{
			PoolPtr pool;
			int priority;
			int error_count;

			Replica() : priority(0), error_count(0) {}
			Replica(PoolPtr pool_, int priority_)
				: pool(pool_), priority(priority_), error_count(0) {}
		};

		typedef std::vector<Replica> Replicas;
		/// [приоритет][номер] -> реплика.
		typedef std::map<int, Replicas> ReplicasByPriority;

		ReplicasByPriority replicas_by_priority;

		/// Количество попыток подключения.
		size_t max_tries;
		/// Mutex для доступа к списку реплик.
		Poco::FastMutex mutex;

	public:
		typedef Pool::Entry Entry;

		/**
		 * @param config_name		Имя параметра в конфигурационном файле.
		 * @param default_connections	Количество подключений по умолчанию к какждой реплике.
		 * @param max_connections	Максимальное количество подключений к какждой реплике.
		 * @param max_tries_		Количество попыток подключения.
		 */
		PoolWithFailover(const std::string & config_name,
			unsigned default_connections = MYSQLXX_POOL_WITH_FAILOVER_DEFAULT_START_CONNECTIONS,
			unsigned max_connections = MYSQLXX_POOL_WITH_FAILOVER_DEFAULT_MAX_CONNECTIONS,
			size_t max_tries_ = MYSQLXX_POOL_WITH_FAILOVER_DEFAULT_MAX_TRIES);

		/** Выделяет соединение для работы. */
		Entry Get();
	};
}
