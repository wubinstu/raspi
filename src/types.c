
#include "head.h"
#include "types.h"

void setPair (KeyValuePair * e, const char * name, const char * value)
{
    strcpy (e->name, name);
    strcpy (e->value, value);
}

void delPair (KeyValuePair * e)
{
    if (e != NULL)
        free (e);
}

void catPair (KeyValuePair e)
{
    printf ("name = %s, value = %s\n", e.name, e.value);
}

void catPairs (KeyValuePair e[], int length)
{
    for (int i = 0; i < length; i++)
        printf ("name = %s, value = %s\n", e[i].name, e[i].value);
}


void clearMonit (RaspiMonitData * mn)
{
    if (mn == NULL)
        return;
    mn->distance = 0;
    mn->cpu_temper = 0;
    mn->env_temper = 0;
    mn->env_humidity = 0;
}

void defaultConfOptClnt (ConfOptClnt * co)
{
    if (co == NULL)
        return;
    co->servIp = inet_addr ("127.0.0.1");
    co->servPort = 9190;
    co->interval = 10;
    co->modeDaemon = true;

    strcpy (co->caFile, "default");
    strcpy (co->pidFile, "default");
}

void defaultConfOptServ (ConfOptServ * co)
{
    if (co == NULL)
        return;
    co->bindIp = INADDR_ANY;
    co->bindPort = 9190;
    co->httpPort = 8080;
    co->sqlPort = 3306;
    co->modeSSL = true;
    co->modeDaemon = true;

    memset (co->caFile, 0, sizeof (co->caFile));
    memset (co->servCert, 0, sizeof (co->servCert));
    memset (co->servKey, 0, sizeof (co->servKey));
    memset (co->pidFile, 0, sizeof (co->pidFile));
    memset (co->sqlHost, 0, sizeof (co->sqlHost));
    memset (co->sqlUser, 0, sizeof (co->sqlUser));
    memset (co->sqlPass, 0, sizeof (co->sqlPass));
    memset (co->sqlName, 0, sizeof (co->sqlName));
}


const int KeyValuePairSize = sizeof (KeyValuePair);
const int LinkNodeSize = sizeof (LinkNode);

void InitLinkList (PLinkNode * L)
{
    (* L) = (PLinkNode) calloc (1, LinkNodeSize);
    (* L)->opt = NULL;
    (* L)->next = NULL;
}

void CreateLinkList (PLinkNode * L, KeyValuePair e[], int length)
{
    PLinkNode master, erratic;
    InitLinkList (L);
    master = (* L);
    // The First LinkNode Will NOT Be Used

    for (int i = 0; i < length; i++)
    {
        erratic = (LinkNode *) calloc (1, LinkNodeSize);
        erratic->opt = (KeyValuePair *) calloc (1, KeyValuePairSize);
        memcpy ((KeyValuePair *) (erratic->opt), (KeyValuePair *) & e[i], KeyValuePairSize);
        erratic->next = NULL;
        master->next = erratic;
        master = erratic;
    }
}

void DestroyLinkList (PLinkNode * L)
{
    PLinkNode master = (* L);
    PLinkNode ahead;

    while (master)
    {
        ahead = master->next;
        free (master);
        master = ahead;
    }
}

int LengthOfLinkList (PLinkNode L)
{
    PLinkNode master = L;
    int length = 0;
    while (master != NULL)
    {
        length++;
        master = master->next;
    }
    return length - 1;
}

void InsertIntoLinkList (PLinkNode * L, int location, KeyValuePair e)
{
    if (location <= 0 || location > LengthOfLinkList (* L))
        return;
    PLinkNode master = * L, erratic;
    int ruler = 1;
    while (ruler < location)
    {
        ruler++;
        master = master->next;
    }
    erratic = calloc (1, LinkNodeSize);
    erratic->opt = calloc (1, KeyValuePairSize);
    erratic->next = NULL;
    memcpy ((KeyValuePair *) (erratic->opt), (KeyValuePair *) & e, KeyValuePairSize);
    erratic->next = master->next;
    master->next = erratic;
}

void AddToLinkList (PLinkNode * L, KeyValuePair e)
{
    PLinkNode master = (* L), erratic;
    while (master->next != NULL)master = master->next;
    erratic = calloc (1, LinkNodeSize);
    erratic->opt = calloc (1, KeyValuePairSize);
    memcpy ((KeyValuePair *) (erratic->opt), (KeyValuePair *) & e, KeyValuePairSize);
    erratic->next = NULL;
    master->next = erratic;
}

void GetFromLinkList (PLinkNode L, int location, KeyValuePair * e)
{
    if (location <= 0 || location > LengthOfLinkList (L))
        return;
    PLinkNode master = L;
    int ruler = 0;
    while (ruler < location)
    {
        ruler++;
        master = master->next;
    }
    memcpy ((KeyValuePair *) e, (KeyValuePair *) (master->opt), KeyValuePairSize);
}

void DeleteAtLinkList (PLinkNode * L, int location)
{
    if (location <= 0 || location > LengthOfLinkList (* L))
        return;
    PLinkNode master = * L;
    PLinkNode ahead = (* L)->next;
    int ruler = 1;
    while (ruler < location)
    {
        ruler++;
        ahead = ahead->next;
        master = master->next;
    }
    master->next = ahead->next;
    free (ahead);
}

void DisplayLinkList (PLinkNode L)
{
    PLinkNode master = L->next;
    while (master != NULL)
    {
        /* calls the output function suitable for a specific element type */
        catPair (* (KeyValuePair *) master->opt);
        master = master->next;
    }
}

int ListToArry (PLinkNode L, KeyValuePair e[])
{
    int i = 0;
    PLinkNode master = L->next;
    while (master != NULL)
    {
        memcpy (& e[i++], master->opt, KeyValuePairSize);
        master = master->next;
    }
    return i;
}


hash_table_client_t * hash_table_client_init (int size)
{
    hash_table_client_t * table = (hash_table_client_t *) calloc (1, sizeof (hash_table_client_t));
    if (table == NULL)return table;
    table->size = size;
    table->current = 0;
    if (pthread_rwlock_init (& table->lock, NULL) != 0)
    {
        free (table);
        return NULL;
    }
    table->hashTable = (hash_node_client_t **) calloc (size, sizeof (hash_node_client_t *));
    return table;
}

void hash_table_client_destroy (hash_table_client_t * table)
{
    if (table == NULL)return;
    pthread_rwlock_wrlock (& table->lock);
    hash_node_client_t * node, * free_node;
    if (table->current > 0 && table->hashTable != NULL)
    {
        for (unsigned int i = 0; i < table->size; i++)
        {
            if (table->hashTable[i] != NULL)
            {
                node = table->hashTable[i];
                while (node != NULL)
                {
                    free_node = node;
                    node = node->next;
                    free (free_node);
                    table->current--;
                }
                table->hashTable[i] = NULL;
            }
        }
        free (table->hashTable);
    }
    table->size = -1;
    pthread_rwlock_destroy (& table->lock);
    free (table);
}

hash_node_client_t * hash_table_client_get (hash_table_client_t * table, int hash_index)
{
    if (table == NULL || table->size <= 0 || table->current <= 0 || table->hashTable == NULL || hash_index < 0)
        return NULL;

    pthread_rwlock_rdlock (& table->lock);
    hash_node_client_t * node = table->hashTable[hash_index % table->size];
    while (node != NULL)
    {
        if (node->hash_node_key == hash_index)
        {
            pthread_rwlock_unlock (& table->lock);
            return node;
        }
        node = node->next;
    }
    pthread_rwlock_unlock (& table->lock);
    return NULL;
}

void hash_table_client_add (hash_table_client_t * table, int hash_index, hash_node_client_t * new_node)
{
    if (table == NULL || table->size <= 0 || table->current >= table->size || table->hashTable == NULL ||
        new_node == NULL)
        return;

    pthread_rwlock_wrlock (& table->lock);
    hash_node_client_t * node = table->hashTable[hash_index % table->size];
    if (node == NULL)
        table->hashTable[hash_index % table->size] = new_node;
    else
    {
        while (node->next != NULL)
            node = node->next;

        node->next = new_node;
        new_node->next = NULL;
    }
    table->current++;
    pthread_rwlock_unlock (& table->lock);
}

void hash_table_client_del (hash_table_client_t * table, int hash_index)
{
    if (table == NULL || table->size <= 0 || table->current <= 0 || table->hashTable == NULL)
        return;

    pthread_rwlock_wrlock (& table->lock);
    hash_node_client_t * free_node;
    hash_node_client_t * node = table->hashTable[hash_index % table->size];

    if (node != NULL)
        if (node->hash_node_key == hash_index)
        {
            table->hashTable[hash_index % table->size] = node->next;
            free (node);
            table->current--;
            pthread_rwlock_unlock (& table->lock);
            return;
        }


    while (node != NULL)
    {
        if (node->next != NULL)
        {
            if (node->next->hash_node_key == hash_index)
            {
                free_node = node->next;
                node->next = free_node->next;
                free (free_node);
                table->current--;
                pthread_rwlock_unlock (& table->lock);
                return;
            }
        }
        node = node->next;
    }
    pthread_rwlock_unlock (& table->lock);
}

hash_table_info_t * hash_table_info_init (int size)
{
    hash_table_info_t * table = (hash_table_info_t *) calloc (1, sizeof (hash_table_info_t));
    if (table == NULL)return table;
    table->size = size;
    table->current = 0;
    if (pthread_rwlock_init (& table->lock, NULL) != 0)
    {
        free (table);
        return NULL;
    }
    table->hashTable = (hash_node_sql_data_t **) calloc (size, sizeof (hash_node_sql_data_t *));
    if (table->hashTable == NULL)
    {
        free (table);
        return NULL;
    }
    return table;
}

void hash_table_info_destroy (hash_table_info_t * table)
{
    if (table == NULL)return;
    hash_node_sql_data_t * node, * free_node;
    pthread_rwlock_wrlock (& table->lock);
    if (table->current > 0)
        for (unsigned int i = 0; i < table->size; i++)
        {
            if (table->hashTable[i] != NULL)
            {
                node = table->hashTable[i];
                while (node != NULL)
                {
                    free_node = node;
                    node = node->next;
                    free (free_node);
                    table->current--;
                }
                table->hashTable[i] = NULL;
            }
        }
    pthread_rwlock_destroy (& table->lock);
    free (table->hashTable);
    table->size = -1;
    free (table);
}

bool hash_table_info_update (hash_table_info_t * table, hash_node_sql_data_t * new_data, struct timespec time)
{
    if (table == NULL || table->size <= 0 || table->hashTable == NULL || new_data == NULL)
        return false;

    // notice that read lock, not write lock
    if (pthread_rwlock_timedrdlock (& table->lock, & time) != 0)
        return false;

    int index = new_data->socket_fd % table->size;
    hash_node_sql_data_t * data = table->hashTable[index];
    hash_node_sql_data_t * pointer = data;

    if (data == NULL)
    {
        table->hashTable[index] = calloc (1, sizeof (hash_node_sql_data_t));
        * table->hashTable[index] = * new_data;
        table->hashTable[index]->next = NULL;
        table->current++;
        pthread_rwlock_unlock (& table->lock);
        return true;
    }

    while (data != NULL)
    {
        if (data->socket_fd == new_data->socket_fd)
        {
            * data = * new_data;
            pthread_rwlock_unlock (& table->lock);
            return true;
        }
        pointer = data;
        data = data->next;
    }

    pointer->next = (hash_node_sql_data_t *) calloc (1, sizeof (hash_node_sql_data_t));
    if (pointer->next == NULL)return false;
    * pointer->next = * new_data;
    table->current++;
    pthread_rwlock_unlock (& table->lock);
    return true;
}

void hash_table_info_delete (hash_table_info_t * table, int hash_index)
{
    if (table == NULL || table->size <= 0 || table->current <= 0 || table->hashTable == NULL || hash_index < 0)
        return;

    pthread_rwlock_wrlock (& table->lock);

    int index = hash_index % table->size;
    hash_node_sql_data_t * data = table->hashTable[index];
    hash_node_sql_data_t * free_node;

    if (data == NULL)
    {
        pthread_rwlock_unlock (& table->lock);
        return;
    }

    if (data->socket_fd == hash_index)
    {
        table->hashTable[index] = data->next;
        free (data);
        table->current--;
        pthread_rwlock_unlock (& table->lock);
        return;
    }

    while (data != NULL)
    {

        if (data->next != NULL)
        {
            if (data->next->socket_fd == hash_index)
            {
                free_node = data->next;
                data->next = free_node->next;
                free (free_node);
                table->current--;
                pthread_rwlock_unlock (& table->lock);
                return;
            }
        }

        data = data->next;
    }
    pthread_rwlock_unlock (& table->lock);
}