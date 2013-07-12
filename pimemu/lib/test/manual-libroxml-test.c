/*******************************************************************************
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 *
 * RESTRICTED RIGHTS NOTICE (DEC 2007)
 * (a)     This computer software is submitted with restricted rights under
 *     Government Contract No. DE-AC52-8MA27344 and subcontract B600716. It
 *     may not be used, reproduced, or disclosed by the Government except as
 *     provided in paragraph (b) of this notice or as otherwise expressly
 *     stated in the contract.
 *
 * (b)     This computer software may be -
 *     (1) Used or copied for use with the computer(s) for which it was
 *         acquired, including use at any Government installation to which
 *         the computer(s) may be transferred;
 *     (2) Used or copied for use with a backup computer if any computer for
 *         which it was acquired is inoperative;
 *     (3) Reproduced for safekeeping (archives) or backup purposes;
 *     (4) Modified, adapted, or combined with other computer software,
 *         provided that the modified, adapted, or combined portions of the
 *         derivative software incorporating any of the delivered, restricted
 *         computer software shall be subject to the same restricted rights;
 *     (5) Disclosed to and reproduced for use by support service contractors
 *         or their subcontractors in accordance with paragraphs (b)(1)
 *         through (4) of this notice; and
 *     (6) Used or copied for use with a replacement computer.
 *
 * (c)     Notwithstanding the foregoing, if this computer software is
 *         copyrighted computer software, it is licensed to the Government with
 *         the minimum rights set forth in paragraph (b) of this notice.
 *
 * (d)     Any other rights or limitations regarding the use, duplication, or
 *     disclosure of this computer software are to be expressly stated in, or
 *     incorporated in, the contract.
 *
 * (e)     This notice shall be marked on any reproduction of this computer
 *     software, in whole or in part.
 ******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <roxml.h>
#include <errno.h>

#define GENERAL_TRY(function, failvar, msg) \
    failvar = function;                     \
    if (failvar != 0)                       \
    {                                       \
        if (msg != NULL)                    \
            fprintf(stderr, msg);           \
        errno = failvar;                    \
        return -1;                          \
    }

int main()
{
    node_t *main, *child, *temp;
    int node_number = 0;
    int node_position = 0;
    int i = 0;
    char node_name[256];
    char node_content[1024];
    char* new_content;
    for(i = 0; i < 256; i++)
        node_name[i]='\0';
    for(i = 0; i < 1024; i++)
        node_content[i] = '\0';
    main = roxml_load_doc("./1.xml");
    main = roxml_get_root(main);
    node_number = roxml_get_type(main);
    node_position = roxml_get_node_position(main);
    roxml_get_name(main, node_name, 256);
    roxml_get_content(main, node_content, 1024, NULL);
    printf("Type: %x Position: %d\n", node_number, node_position);
    printf("Node name: %s\n", node_name);
    printf("Content: %s\n", node_content);
    i = roxml_get_chld_nb(main);
    printf("Number of children: %d\n\n", i);

    // Get the first child
    child = roxml_get_chld(main, NULL, 0);
    node_number = roxml_get_type(child);
    roxml_get_name(child, node_name, 256);
    printf("First Child name: %s\n\n", node_name);
    child = roxml_get_chld(child, NULL, 0);
    child = roxml_get_chld(child, NULL, 0);
    node_number = roxml_get_type(child);
    node_position = roxml_get_node_position(child);
    roxml_get_name(child, node_name, 256);
    new_content = roxml_get_content(child, node_content, 1024, NULL);
    printf("Type: %x Position: %d\n", node_number, node_position);
    printf("2nd to Final child name: %s\n", node_name);
    printf("Child content: %s\n", node_content);
    printf("Returned content: %s\n", new_content);
    i = roxml_get_chld_nb(child);
    printf("Number of children: %d\n\n", i);

    temp = roxml_get_txt(child, 0);
    //roxml_get_content(temp

    child = roxml_get_next_sibling(child);

    node_number = roxml_get_type(child);
    node_position = roxml_get_node_position(child);
    roxml_get_name(child, node_name, 256);
    roxml_get_content(child, node_content, 1024, NULL);
    printf("Type: %x Position: %d\n", node_number, node_position);
    printf("Final child name: %s\n", node_name);
    printf("Child content: %s\n", node_content);

    i = roxml_get_chld_nb(child);
    printf("Number of children: %d\n", i);

}
