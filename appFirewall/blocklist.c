
#include "blocklist.h"

// globals
static bl_item_t block_list[MAXBLOCKLIST];
static int block_list_size=0;
Hashtable *bl_htab=NULL; // hash table of pointers into block list for fast lookup

bl_item_t conn_to_bl_item(conn_info_t item) {
		bl_item_t bl;
		strcpy(bl.name, item.name);
		strcpy(bl.addr_name, item.addr_name);
		bl.af = item.af;
		memcpy(&bl.addr, &item.addr, sizeof(bl.addr));
		strcpy(bl.domain, item.domain);
		return bl;
}

int on_blocklist(bl_item_t item) {
	int i;
	for (i=0; i<block_list_size; i++) {
		if (strcmp(block_list[i].name, item.name) !=0 )
			continue;
		//printf("matched names %s %s ... ",block_list[i].name,item.name);
		//printf("af=%d/%d ...", block_list[i].af, item.af);
		//char sn[INET6_ADDRSTRLEN], dn[INET6_ADDRSTRLEN];
		//printf("addr=%s %s\n",inet_ntop(AF_INET, &block_list[i].addr.s6_addr, sn, INET6_ADDRSTRLEN), inet_ntop(AF_INET, &item.addr.s6_addr, dn, INET6_ADDRSTRLEN));
		if (block_list[i].af != item.af)
			continue;
		if (are_addr_same(item.af,&block_list[i].addr,&item.addr))
			return i;
		/*if (memcmp(&block_list[i].addr.s6_addr,&item.addr.s6_addr,16)) {
			continue;
		}
		//printf("addr match\n");
		return i;*/
	}
	return -1;
}

char* hash_item(const bl_item_t *item) {
	// generate table lookup key string from block list item PID name and dest address
	char* temp = malloc(strlen(item->name)+strlen(item->addr_name_clean)+1);
	strcpy(temp,item->name);
	strcat(temp,item->addr_name_clean);
	return temp;
}

bl_item_t * in_blocklist_htab(const bl_item_t *item) {
	if (block_list_size>0) {
		char *temp = hash_item(item);
		bl_item_t * res = hashtable_get(bl_htab, temp);
		free(temp);
		return res;
	} else
		return NULL;
}

void add_blockitem_to_htab(bl_item_t *item) {
	// add item to hash table
	// make sure we have a simple string version of address ...
	inet_ntop(item->af, &item->addr, item->addr_name_clean, INET6_ADDRSTRLEN);
	// construct key string
	char * temp = hash_item(item);
	hashtable_put(bl_htab, temp, item);
	free(temp);
}

bl_item_t create_blockitem_from_addr(conn_raw_t *cr, int udp) {
	// create a new blocklist item from raw connection info (assumed to be
	// outgoing connection, so src is local and dst is remote)
	bl_item_t c;
	memset(&c,0,sizeof(c));

	//clock_t begin = clock();

	c.af=cr->af;
	// get human readable form of dest adddr
	inet_ntop(cr->af, &cr->dst_addr, c.addr_name_clean, INET6_ADDRSTRLEN);
	// and keep copy of binary form too
	if (cr->af==AF_INET)
		memcpy(&c.addr,&cr->dst_addr,sizeof(struct in_addr));
	else
		memcpy(&c.addr,&cr->dst_addr,sizeof(struct in6_addr));
	
	// try to get PID info
	int res=find_pid(cr,c.name, udp);
	//clock_t end1 = clock();
	if (res==0) {
		strcpy(c.name,"<unknown>");
	}
	// try to get domain name
	char* dns =lookup_dns_name(c.af, c.addr);
	if (dns!=NULL) {
		strcpy(c.domain,dns);
	}

	//clock_t end = clock();
	//printf("pid2 %f/%f ... ",(end1 - begin)*1.0 / CLOCKS_PER_SEC,(end - begin)*1.0 / CLOCKS_PER_SEC);
	
	// and store long form of dest addr (i.e. addr plus dns name)
	// for use in displaying on GUI etc
	char dns2[BUFSIZE]={0};
	if (strlen(c.domain)) {
		sprintf(dns2," (%s)",c.domain);
	}
	sprintf(c.addr_name,"%s%s",c.addr_name_clean,dns2);

	return c;
}

void del_blockitem_from_htab(bl_item_t *item) {
	char * temp = hash_item(item);
	hashtable_remove(bl_htab, temp);
	free(temp);
}

void add_blockitem(bl_item_t item) {
	//INFO("add_blockitem\n");
	//char str[256];
	//printf("%s %s\n", item.name, inet_ntop(item.af, &item.addr.s6_addr, str, INET6_ADDRSTRLEN));
	//return;
	if (on_blocklist(item)>=0) {
		WARN("add_blockitem() item exists.\n");
		return;
	}
	if (block_list_size < MAXBLOCKLIST) {
		block_list[block_list_size] = item;
		DEBUG2("%d %s %s\n", block_list_size, item.name, block_list[block_list_size].name);
		//INFO("%s %s\n", item.addr_name, block_list[block_list_size].addr_name);
		block_list_size++;
	} else {
		WARN("add_blockitem() list full.\n");
	}
	add_blockitem_to_htab(&item);
}

int del_blockitem(bl_item_t item) {
	int i,posn;
	if ((posn=on_blocklist(item))<0) {
		WARN("del_blockitem() item not found.\n");
		return -1;
	}
	for (i=posn; i<block_list_size-1; i++) {
		block_list[i] = block_list[i+1];
	}
	block_list_size--;
	del_blockitem_from_htab(&item);
	return 0;
}

int get_blocklist_size(void) {
	return block_list_size;
}

bl_item_t get_blocklist_item(int row) {
	return block_list[row];
}

void save_blocklist(void) {
	//printf("saving block_list\n");
	char path[1024]; strcpy(path,get_path());
	FILE *fp = fopen(strcat(path,BLOCKLISTFILE),"w");
	//char cwd[1024];
	//getcwd(cwd, sizeof(cwd));
	//printf("Current working dir: %s\n", cwd);
	if (fp==NULL) {
		WARN("Problem opening %s for writing: %s\n", BLOCKLISTFILE, strerror(errno));
		return;
	}
	int i;
	int res = (int)fwrite(&block_list_size,sizeof(block_list_size),1,fp);
	if (res<1) {
		WARN("Problem saving size to %s: %s\n", BLOCKLISTFILE,strerror(errno));
		return;
	}
	for(i = 0; i < block_list_size; i++){
		int res=(int)fwrite(&block_list[i],sizeof(bl_item_t),1,fp);
		if (res<1) {
			WARN("Problem saving %s: %s\n", BLOCKLISTFILE, strerror(errno));
			break;
		}
	}
	fclose(fp);
}

void load_blocklist(void) {
	
	// initialise hash table
	if (bl_htab!=NULL) hashtable_free(bl_htab);
	bl_htab = hashtable_new(MAXBLOCKLIST);
	block_list_size = 0;
	//return;

	// open and read file
	char path[1024]; strcpy(path,get_path());
	FILE *fp = fopen(strcat(path,BLOCKLISTFILE),"r");
	if (fp==NULL) {
		WARN("Problem opening %s for reading: %s\n", BLOCKLISTFILE, strerror(errno));
		return;
	}
	fread(&block_list_size,sizeof(block_list_size),1,fp);
	//printf("block_list_size=%d\n",block_list_size);
	int i;
	for(i = 0; i < block_list_size; i++){
		int res=(int)fread(&block_list[i],sizeof(bl_item_t),1,fp);
		if (res<1) {
			WARN("Problem loading %s: %s", BLOCKLISTFILE, strerror(errno));
			//block_list_size=0;
			break;
		}
		// and put pointer into hash table
		add_blockitem_to_htab(&block_list[i]);
	}
	if (i<block_list_size) {
		WARN("Read too few records from %s: expected %d, got %d\n",BLOCKLISTFILE,block_list_size,i);
		block_list_size = i;
	}
	fclose(fp);
}
