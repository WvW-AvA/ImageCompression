#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"
#include "log.h"
void print_priority_queue(huffman_node *node)
{
    if (node->son0 == NULL && node->son1 == NULL)
        printf("(%d,%d)", node->value, node->frequency);
    else
        printf("(,%d)", node->frequency);
    if (node->before != NULL)
    {
        printf("-->");
        print_priority_queue(node->before);
    }
    else
    {
        printf("\n");
    }
}

void print_queue(huffman_node *node, uint32_t size)
{
    for (int i = 0; i < size; i++)
    {
        if (node[i].son0 == NULL && node[i].son1 == NULL)
            printf("%d:(%d,%d)-->", i, node[i].value, node[i].frequency);
        else
            printf("%d:(,%d)-->", i, node[i].frequency);
    }
    printf("\n");
}
void print_haffman_tree(huffman_node *node)
{
    static int stack_ind = 0;
    stack_ind++;
    for (int i = 0; i < stack_ind; i++)
        printf("-");
    if (node->son0 != NULL && node->son1 != NULL)
    {
        printf(GREEN "(,%d,%d)\n" RESET, node->frequency, node->code);
        print_haffman_tree(node->son0);
        print_haffman_tree(node->son1);
    }
    else
        printf(RED "(%d,%d,%d)\n" RESET, node->value, node->frequency, node->code);
    stack_ind--;
}

void find_father(huffman_node *node, huffman_node *father, uint8_t code)
{
    node->before = father;
    node->code = code;
    if (node->son0 != NULL && node->son1 != NULL)
    {
        find_father(node->son0, node, 0);
        find_father(node->son1, node, 1);
    }
}

uint32_t build_huffman_tree(huffman_node *nodes, uint16_t symbol_num)
{
    huffman_node *end_node = nodes + symbol_num - 1;
    huffman_node *curr_node = end_node;
    uint32_t ind = symbol_num;
    while (curr_node != NULL && curr_node->before != NULL)
    {
        nodes[ind].frequency = curr_node->frequency + curr_node->before->frequency;
        nodes[ind].son0 = curr_node;
        nodes[ind].son1 = curr_node->before;
        // insert priority_queue
        huffman_node *p = curr_node->before, *next = curr_node;
        while (p && p->frequency < nodes[ind].frequency)
        {
            next = p;
            p = p->before;
        }
        nodes[ind].before = p;
        next->before = nodes + ind;

        curr_node = curr_node->before->before;
        ind++;
    }
    ind--;
    find_father(&nodes[ind], NULL, 2);
    print_haffman_tree(&nodes[ind]);
    return ind;
}

huffman_encode_handle huffman_encode(huffman_decode_handle src)
{
    huffman_encode_handle ret;
    huffman_statstic_struct *symbol_group = (huffman_statstic_struct *)calloc(256, sizeof(huffman_statstic_struct));
    huffman_node *nodes = (huffman_node *)calloc(512, sizeof(huffman_node));

    uint32_t *buffer = (uint32_t *)calloc(src.size, sizeof(uint32_t) / 2);
    uint32_t buffer_index = 0;
    for (int i = 0; i < 256; i++)
    {
        symbol_group[i].frequency = 0;
        symbol_group[i].value = i;
    }
    // statistic frequency of each symbol
    for (int i = 0; i < src.size; i++)
        symbol_group[src.data[i]].frequency++;
    // copy to nodes
    for (int i = 0; i < 256; i++)
    {
        nodes[i].frequency = symbol_group[i].frequency;
        nodes[i].value = symbol_group[i].value;
    }
    // sort
    huffman_node temp;
    for (int i = 0; i < 255; i++)
    {
        for (int j = 0; j < 255; j++)
        {
            if (nodes[j].frequency < nodes[j + 1].frequency)
            {
                temp = nodes[j];
                nodes[j] = nodes[j + 1];
                nodes[j + 1] = temp;
            }
        }
    }
    // build huffman tree
    uint16_t symbol_num = 0;
    for (symbol_num = 0; symbol_num < 256; symbol_num++)
    {
        if (nodes[symbol_num].frequency == 0)
            break;
        LOG("%d symbol:%d frequency: %d", symbol_num, nodes[symbol_num].value, nodes[symbol_num].frequency);
        symbol_group[nodes[symbol_num].value].point = &nodes[symbol_num];
        if (symbol_num != 0)
            nodes[symbol_num].before = nodes + symbol_num - 1;
    }
    LOG("Print priority queue");
    print_priority_queue(&nodes[symbol_num - 1]);
    int root_index = build_huffman_tree(nodes, symbol_num);

    // encode
    huffman_node *curr;
    uint8_t index;
    uint32_t stack[25];
    int stack_index;
    for (int i = 0; i < src.size; i++)
    {
        curr = symbol_group[src.data[i]].point;
        stack_index = 0;
        while (curr->code != 2)
        {
            stack[stack_index] = curr->code;
            stack_index++;
            curr = curr->before;
        }
        for (stack_index--; stack_index >= 0; stack_index--)
        {
            if (index >= 32)
            {
                buffer_index++;
                index = 0;
            }
            buffer[buffer_index] = buffer[buffer_index] | (stack[stack_index] << (31 - index));
            index++;
        }
    }

    // pack data
    ret.data_size = (buffer_index + 1) * 4;
    ret.symbol_num = symbol_num;
    ret.origin_size = src.size;
    ret.last_byte_record = index;
    ret.file_size = ret.symbol_num * 5 + ret.data_size + 11;
    ret.raw = (uint8_t *)malloc(ret.file_size * sizeof(uint8_t));
    ret.symbol = ret.raw + 11;
    ret.symbol_freq = (uint32_t *)(ret.symbol + ret.symbol_num);
    ret.data = (uint8_t *)(ret.symbol_freq + ret.symbol_num);
    *(uint16_t *)(&ret.raw[0]) = ret.symbol_num;
    *((uint32_t *)(&ret.raw[2])) = ret.origin_size;
    *((uint32_t *)(&ret.raw[6])) = ret.data_size;
    ret.raw[10] = ret.last_byte_record;
    memcpy(ret.data, buffer, ret.data_size);
    for (int i = 0; i < symbol_num; i++)
    {
        ret.symbol[i] = nodes[i].value;
        ret.symbol_freq[i] = nodes[i].frequency;
    }
    free(nodes);
    free(symbol_group);
    free(buffer);
    LOG("huffman encode done!");
    return ret;
}

huffman_decode_handle huffman_decode(huffman_encode_handle src)
{
    huffman_decode_handle ret;
    ret.size = src.origin_size;
    ret.data = (uint8_t *)malloc((ret.size + 2) * sizeof(uint8_t));
    // build huffman tree
    huffman_node nodes[512];
    for (int i = 0; i < src.symbol_num; i++)
    {
        nodes[i].value = src.symbol[i];
        nodes[i].frequency = src.symbol_freq[i];
        if (i != 0)
            nodes[i].before = &nodes[i - 1];
    }
    uint32_t root_index = build_huffman_tree(nodes, src.symbol_num);

    huffman_node *root = nodes + root_index;
    huffman_node *curr = root;
    uint32_t index = 0;
    uint8_t bit_index = 0;
    uint32_t *data = (uint32_t *)src.data;
    for (int i = 0; i < (src.data_size / 4); i++)
    {
        uint8_t temp = ((i == (src.data_size / 4 - 1)) ? src.last_byte_record : 32);
        for (int j = 0; j < temp; j++)
        {
            if (curr->son0 != NULL && curr->son0 != NULL)
            {
                if (data[i] & (0x80000000 >> j))
                    curr = curr->son1;
                else
                    curr = curr->son0;
            }
            if (curr->son0 == NULL && curr->son1 == NULL)
            {
                ret.data[index] = curr->value;
                curr = root;
                index++;
            }
        }
    }

    // LOG("%d", index);
    // for (int i = 0; i < index; i++)
    //     printf("%d ", ret.data[i]);
    // printf("\n");
    if (index != ret.size)
    {
        LOG_ERROR("buffman decode error: decode size is not equal to origin size");
    }
    LOG("huffman decode done, decode size:%d", ret.size);
    return ret;
}

void huffman_save(huffman_encode_handle src, const char *path)
{
    FILE *f = fopen(path, "w");
    fwrite(src.raw, src.file_size, 1, f);
    fclose(f);
    LOG("Save huffman file %s", path);
}

void huffman_map(huffman_encode_handle *target)
{
    target->symbol_num = *((uint16_t *)(target->raw));
    target->origin_size = *((uint32_t *)(target->raw + 2));
    target->data_size = *((uint32_t *)(target->raw + 6));
    target->last_byte_record = *(target->raw + 10);
    target->symbol = target->raw + 11;
    target->symbol_freq = (uint32_t *)(target->symbol + target->symbol_num);
    target->data = (uint8_t *)(target->symbol_freq + target->symbol_num);
    LOG("symbol num:%d \n origin size:%d \n data size %d \n last byte recode:%d", target->symbol_num, target->origin_size, target->data_size, target->last_byte_record);
}
huffman_encode_handle huffman_load(const char *path)
{
    huffman_encode_handle ret;
    FILE *f = fopen(path, "r");
    fseek(f, 0L, SEEK_END);
    uint32_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    ret.raw = malloc(size * sizeof(uint8_t));
    fread(ret.raw, size, 1, f);
    fclose(f);
    ret.file_size = size;
    LOG("Load huffman file %s  file size:%d", path, size);
    huffman_map(&ret);
    return ret;
}
