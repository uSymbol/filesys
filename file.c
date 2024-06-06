//
// Created by 84424 on 2024/6/6.
//

#include "file.h"

storage_file_s *storage_file_init(char *filename, int max_num, int key_len, int value_len)
{
    i8 ver[16] = {0x01,0x0A,0x02,0x0B,0x03,0x0C,0x04,0x0D,0x20,0x24,0x06,0x06,0x11,0x22,0x33,0x44};

    FILE *fp = NULL;

    storage_file_s *handler = malloc(sizeof(storage_file_s));
    if (handler == NULL)
    {
        return NULL;
    }

    int exist = access(filename,F_OK);

    if (exist == -1)
    {
        // 不存在
        fp = fopen(filename,"w+");
        if (fp == NULL)
        {
            free(handler);
            return NULL;
        }

        handler->fp = fp;
        memcpy(handler->header.version,ver,16);
        handler->header.timestamp = time(NULL);
        handler->header.item_num_max = max_num;
        handler->header.item_num_cur = 0;
        handler->header.item_len = value_len;
        handler->header.sizeof_key = key_len;

        handler->items = malloc(handler->header.item_num_max * (sizeof(item_header_s) + key_len));
        if (handler->items == NULL)
        {
            fclose(fp);
            free(handler);
            return NULL;
        }
        memset(handler->items,0x00,handler->header.item_num_max * (sizeof(item_header_s) + key_len));

        for(i32 i = 0; i < handler->header.item_num_max; i++)
        {
            item_header_s *item = (item_header_s *) (handler->items + (i * (sizeof(item_header_s) + key_len)));
            item->offset = sizeof(file_header_s) + max_num * (sizeof(item_header_s) + key_len) + i * value_len;
            item->len = (i16)key_len;
            item->full = 0;
        }

        fwrite(&(handler->header), sizeof(file_header_s),1,fp);
        fwrite(handler->items, handler->header.item_num_max * (sizeof(item_header_s) + key_len),1,fp);

        return handler;
    }
    else
    {
        // 存在
        fp = fopen(filename,"rb+");
        if (fp == NULL)
        {
            free(handler);
            return NULL;
        }

        fseek(fp,0,SEEK_END);
        i64 size = ftell(fp);
        rewind(fp);

        if (size < sizeof(file_header_s) + max_num * (sizeof(item_header_s) + key_len))
        {
            fclose(fp);
            free(handler);
            return NULL;
        }

        handler->fp = fp;

        fread(&handler->header,sizeof(file_header_s),1,fp);

        if (memcmp(&handler->header.version, ver, 16) != 0)
        {
            fclose(fp);
            free(handler);
            return NULL;
        }

        if (handler->header.item_num_max != max_num)
        {
            fclose(fp);
            free(handler);
            return NULL;
        }

        if (handler->header.item_len != value_len)
        {
            fclose(fp);
            free(handler);
            return NULL;
        }

        if (handler->header.sizeof_key != key_len)
        {
            fclose(fp);
            free(handler);
            return NULL;
        }

        handler->items = malloc(max_num * (sizeof(item_header_s) + key_len));
        if (handler->items == NULL)
        {
            fclose(fp);
            free(handler);
            return NULL;
        }

        fread(handler->items, max_num * (sizeof(item_header_s) + key_len), 1, fp);

        return handler;
    }
}

int storage_insert(storage_file_s *obj, void *key, void *value, int value_len)
{
    if (value_len > obj->header.item_len)
    {
        return 0;
    }

    i8 ok = 0;

    for (int i = 0; i < obj->header.item_num_max; i++)
    {
        item_header_s *item_header = (item_header_s *) (obj->items + (i * (sizeof(item_header_s) + obj->header.sizeof_key)));

        printf("[debug] %s:%d ",__FILE__,__LINE__);
        printf("offset:%llu full:%d len:%d \n",item_header->offset,item_header->full,item_header->len);

        if (item_header->full == 0)
        {
            printf("[debug] %s:%d \n",__FILE__,__LINE__);
            item_s *item = (item_s *) item_header;
            printf("[debug] %s:%d \n",__FILE__,__LINE__);
            memcpy(item->key, key, item_header->len);

            printf("[debug] %s:%d \n",__FILE__,__LINE__);

            u8 data[obj->header.item_len];
            memset(data,0x00,obj->header.item_len);
            memcpy(data,value,value_len);

            printf("[debug] %s:%d \n",__FILE__,__LINE__);

            fseek(obj->fp, item_header->offset, SEEK_SET);

            printf("[debug] %s:%d \n",__FILE__,__LINE__);

            fwrite(data,obj->header.item_len,1,obj->fp);

            printf("[debug] %s:%d \n",__FILE__,__LINE__);

            item_header->full = 1;
            fseek(obj->fp, sizeof(file_header_s) + i * (sizeof(item_header_s) + obj->header.sizeof_key), SEEK_SET);

            printf("[debug] %s:%d \n",__FILE__,__LINE__);

            fwrite(item, (sizeof(item_header_s) + obj->header.sizeof_key), 1, obj->fp);

            printf("[debug] %s:%d \n",__FILE__,__LINE__);

            ok = 1;
        }
    }

    if (ok)
    {
        return 1;
    }

    return 0;
}

int storage_delete(storage_file_s *obj, void *key)
{
    for (int i = 0; i < obj->header.item_num_max; i++)
    {
        item_header_s *item_header = (item_header_s *) (obj->items + (i * (sizeof(item_header_s) + obj->header.sizeof_key)));
        if (item_header->full)
        {
            item_s *item = (item_s *) item_header;
            memset(item->key, 0x00, obj->header.sizeof_key);

            item_header->full = 0;
            fseek(obj->fp, sizeof(file_header_s) + i * (sizeof(item_header_s) + obj->header.sizeof_key), SEEK_SET);
            fwrite(item, (sizeof(item_header_s) + obj->header.sizeof_key), 1, obj->fp);
        }
    }
    return 1;
}


int storage_update(storage_file_s *obj, void *key, void *value, int value_len)
{
    if (value_len > obj->header.item_len)
    {
        return 0;
    }

    i8 ok = 0;

    for (int i = 0; i < obj->header.item_num_max; i++)
    {
        item_header_s *item_header = (item_header_s *) (obj->items + (i * (sizeof(item_header_s) + obj->header.sizeof_key)));
        if (item_header->full)
        {
            item_s *item = (item_s *) item_header;
            if (memcmp(item->key, key, obj->header.sizeof_key) == 0)
            {
                u8 data[obj->header.item_len];
                memset(data,0x00,obj->header.item_len);
                memcpy(data,value,value_len);

                fseek(obj->fp, item_header->offset, SEEK_SET);
                fwrite(data,obj->header.item_len,1,obj->fp);
                ok = 1;
            }
        }
    }

    if (ok)
    {
        return 1;
    }

    return 0;
}


int storage_find(storage_file_s *obj, void *key, void *value, int value_len)
{
    i8 ok = 0;

    for (int i = 0; i < obj->header.item_num_max; i++)
    {
        item_header_s *item_header = (item_header_s *) (obj->items + (i * (sizeof(item_header_s) + obj->header.sizeof_key)));
        if (item_header->full)
        {
            item_s *item = (item_s *) item_header;
            if (memcmp(item->key, key, obj->header.sizeof_key) == 0)
            {
                fseek(obj->fp, item_header->offset, SEEK_SET);
                fread(value, (value_len > obj->header.item_len ? obj->header.item_len : value_len), 1, obj->fp);
                ok = 1;
            }
        }
    }

    if (ok)
    {
        return 1;
    }

    return 0;
}
