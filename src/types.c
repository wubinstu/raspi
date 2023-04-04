
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


hash_table_t * hash_table_init (int size)
{
    hash_table_t * table = (hash_table_t *) calloc (1, sizeof (hash_table_t));
    if (table == NULL)return table;
    table->size = size;
    table->current = 0;
    table->hashTable = (hash_node_client_t **) calloc (size, sizeof (hash_node_client_t *));
    return table;
}

void hash_table_destroy (hash_table_t * table)
{
    if (table == NULL)return;
    hash_node_client_t * node, * free_node;
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
    free (table->hashTable);
    table->size = -1;
    free (table);
}

hash_node_client_t * hash_table_get (hash_table_t * table, int hash_index)
{
    if (table == NULL || table->current <= 0 || hash_index < 0)
        return NULL;

    hash_node_client_t * node = table->hashTable[hash_index % table->size];
    while (node != NULL)
    {
        if (node->hash_node_key == hash_index)
            return node;
        node = node->next;
    }
    return NULL;
}

void hash_table_add (hash_table_t * table, int hash_index, hash_node_client_t * new_node)
{
    if (table == NULL || table->current >= table->size || new_node == NULL)
        return;

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
}

void hash_table_del (hash_table_t * table, int hash_index, int hash_key)
{
    if (table == NULL || table->current <= 0 || hash_index < 0 || hash_key < 0)
        return;

    hash_node_client_t * free_node;
    hash_node_client_t * node = table->hashTable[hash_index % table->size];

    if (node != NULL)
        if (node->hash_node_key == hash_key)
        {
            table->hashTable[hash_index % table->size] = node->next;
            free (node);
            table->current--;
            return;
        }


    while (node != NULL)
    {
        if (node->next != NULL)
        {
            if (node->next->hash_node_key == hash_key)
            {
                free_node = node->next;
                node->next = free_node->next;
                free (free_node);
                table->current--;
                return;
            }
        }
        node = node->next;
    }
}