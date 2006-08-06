#include <stdio.h>
#include <stdlib.h>
#include <alpm.h>

void cleanup(int ret)
{
	alpm_trans_release();
	alpm_release();
	exit(ret);
}

int main(void)
{
    PM_DB *db_local;
    PM_LIST *lp;
    
    if(alpm_initialize("/") == -1)
	{
		fprintf(stderr, "failed to initilize alpm library (%s)\n",
			alpm_strerror(pm_errno));
		return(1);
	}
	
	if(alpm_set_option(PM_OPT_DBPATH, (long)PM_DBPATH) == -1)
	{
		fprintf(stderr, "failed to set option DBPATH (%s)\n",
			alpm_strerror(pm_errno));
		cleanup(1);
	}
	
	db_local = alpm_db_register("local");
	if(db_local == NULL)
	{
		fprintf(stderr, "could not register 'local' database (%s)\n",
			alpm_strerror(pm_errno));
		return(1);
	}
	
	for(lp = alpm_db_getgrpcache(db_local); lp; lp = alpm_list_next(lp))
	{
	   PM_GRP *grp = alpm_list_getdata(lp);
	   printf("%s\n", (char *)alpm_grp_getinfo(grp,PM_GRP_NAME));
	}
	
	alpm_release();
}
