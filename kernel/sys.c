/*
 *  linux/kernel/sys.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <errno.h>

#include <linux/sched.h>
#include <linux/tty.h>
#include <linux/kernel.h>
#include <asm/segment.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <ldir.h>
#include <string.h>

int sys_ftime()
{
	return -ENOSYS;
}

int sys_break()
{
	return -ENOSYS;
}

int sys_ptrace()
{
	return -ENOSYS;
}

int sys_stty()
{
	return -ENOSYS;
}

int sys_gtty()
{
	return -ENOSYS;
}

int sys_rename()
{
	return -ENOSYS;
}

int sys_prof()
{
	return -ENOSYS;
}

int sys_setregid(int rgid, int egid)
{
	if (rgid>0) {
		if ((current->gid == rgid) || 
		    suser())
			current->gid = rgid;
		else
			return(-EPERM);
	}
	if (egid>0) {
		if ((current->gid == egid) ||
		    (current->egid == egid) ||
		    suser()) {
			current->egid = egid;
			current->sgid = egid;
		} else
			return(-EPERM);
	}
	return 0;
}

int sys_setgid(int gid)
{
/*	return(sys_setregid(gid, gid)); */
	if (suser())
		current->gid = current->egid = current->sgid = gid;
	else if ((gid == current->gid) || (gid == current->sgid))
		current->egid = gid;
	else
		return -EPERM;
	return 0;
}

int sys_acct()
{
	return -ENOSYS;
}

int sys_phys()
{
	return -ENOSYS;
}

int sys_lock()
{
	return -ENOSYS;
}

int sys_mpx()
{
	return -ENOSYS;
}

int sys_ulimit()
{
	return -ENOSYS;
}

int sys_time(long * tloc)
{
	int i;

	i = CURRENT_TIME;
	if (tloc) {
		verify_area(tloc,4);
		put_fs_long(i,(unsigned long *)tloc);
	}
	return i;
}

/*
 * Unprivileged users may change the real user id to the effective uid
 * or vice versa.
 */
int sys_setreuid(int ruid, int euid)
{
	int old_ruid = current->uid;
	
	if (ruid>0) {
		if ((current->euid==ruid) ||
                    (old_ruid == ruid) ||
		    suser())
			current->uid = ruid;
		else
			return(-EPERM);
	}
	if (euid>0) {
		if ((old_ruid == euid) ||
                    (current->euid == euid) ||
		    suser()) {
			current->euid = euid;
			current->suid = euid;
		} else {
			current->uid = old_ruid;
			return(-EPERM);
		}
	}
	return 0;
}

int sys_setuid(int uid)
{
/*	return(sys_setreuid(uid, uid)); */
	if (suser())
		current->uid = current->euid = current->suid = uid;
	else if ((uid == current->uid) || (uid == current->suid))
		current->euid = uid;
	else
		return -EPERM;
	return(0);
}

int sys_stime(long * tptr)
{
	if (!suser())
		return -EPERM;
	startup_time = get_fs_long((unsigned long *)tptr) - jiffies/HZ;
	return 0;
}

int sys_times(struct tms * tbuf)
{
	if (tbuf) {
		verify_area(tbuf,sizeof *tbuf);
		put_fs_long(current->utime,(unsigned long *)&tbuf->tms_utime);
		put_fs_long(current->stime,(unsigned long *)&tbuf->tms_stime);
		put_fs_long(current->cutime,(unsigned long *)&tbuf->tms_cutime);
		put_fs_long(current->cstime,(unsigned long *)&tbuf->tms_cstime);
	}
	return jiffies;
}

int sys_brk(unsigned long end_data_seg)
{
	if (end_data_seg >= current->end_code &&
	    end_data_seg < current->start_stack - 16384)
		current->brk = end_data_seg;
	return current->brk;
}

/*
 * This needs some heave checking ...
 * I just haven't get the stomach for it. I also don't fully
 * understand sessions/pgrp etc. Let somebody who does explain it.
 */
int sys_setpgid(int pid, int pgid)
{
	int i;

	if (!pid)
		pid = current->pid;
	if (!pgid)
		pgid = current->pid;
	for (i=0 ; i<NR_TASKS ; i++)
		if (task[i] && task[i]->pid==pid) {
			if (task[i]->leader)
				return -EPERM;
			if (task[i]->session != current->session)
				return -EPERM;
			task[i]->pgrp = pgid;
			return 0;
		}
	return -ESRCH;
}

int sys_getpgrp(void)
{
	return current->pgrp;
}

int sys_setsid(void)
{
	if (current->leader && !suser())
		return -EPERM;
	current->leader = 1;
	current->session = current->pgrp = current->pid;
	current->tty = -1;
	return current->pgrp;
}

int sys_getgroups()
{
	return -ENOSYS;
}

int sys_setgroups()
{
	return -ENOSYS;
}

int sys_uname(struct utsname * name)
{
	static struct utsname thisname = {
		"linux .0","nodename","release ","version ","machine "
	};
	int i;

	if (!name) return -ERROR;
	verify_area(name,sizeof *name);
	for(i=0;i<sizeof *name;i++)
		put_fs_byte(((char *) &thisname)[i],i+(char *) name);
	return 0;
}

int sys_sethostname()
{
	return -ENOSYS;
}

int sys_getrlimit()
{
	return -ENOSYS;
}

int sys_setrlimit()
{
	return -ENOSYS;
}

int sys_getrusage()
{
	return -ENOSYS;
}

int sys_gettimeofday()
{
	return -ENOSYS;
}

int sys_settimeofday()
{
	return -ENOSYS;
}


int sys_umask(int mask)
{
	int old = current->umask;

	current->umask = mask & 0777;
	return (old);
}


int sys_getdents(unsigned int fd, struct linux_dirent *dirp, unsigned int count)
{
	struct m_inode *curinode=current->filp[fd]->f_inode;
	struct linux_dirent mydirent;
	struct buffer_head *bk;
	struct dir_entry *dr;
	char *buf;
	int num = 0,k,block;
	int len_dir = sizeof(struct dir_entry);
	int len_dirent = sizeof(struct linux_dirent);

	if (!(block = curinode->i_zone[0]))
		return -1;
	if (!(bk = bread(curinode->i_dev,block)))
		return -1;

	dr = (struct dir_entry *) bk->b_data;
	
	for (k = 0; k < curinode->i_size; k += len_dir)
	{
		if (num + len_dirent > count) break;
		if ((char *)dr >= BLOCK_SIZE + bk->b_data) {
			brelse(bk);
			bk = NULL;
			if (!(block = bmap(curinode, k/DIR_ENTRIES_PER_BLOCK)) ||
				!(bk = bread(curinode->i_dev, block))) {
				k += DIR_ENTRIES_PER_BLOCK;
				continue;
			}
			dr = (struct dir_entry *) bk->b_data;
		}
		if (dr->inode)
		{
			mydirent.d_ino = dr->inode;
			strcpy(mydirent.d_name,dr->name);
			mydirent.d_off = 0;
			mydirent.d_reclen = sizeof(mydirent);
			buf = &mydirent;
			int i;
			for (i = 0; i < mydirent.d_reclen; i++)
			{
				put_fs_byte(*(buf + i), ((char *)dirp) + i + num);
			}
			num += mydirent.d_reclen;
		}
		dr++;
	}
	brelse(bk);
	return num;
}

int sys_sleep(unsigned int seconds)
{
	sys_signal(14,1,0);
	sys_alarm(seconds);
	sys_pause();
	return 0;
}


long sys_getcwd(char * buf, size_t size)
{
	int cur_inode, fa_inode, ff_inode,k; 
	struct m_inode *cur_dr=current->pwd,*root_dr=current->root;
	struct buffer_head *bk;
	struct dir_entry *dr;
	char tmp[256]={0}, ans_buf[256]={0};
	int len_dir = sizeof(struct dir_entry);

	if(cur_dr!=root_dr){
		bk = bread(current->pwd->i_dev,cur_dr->i_zone[0]);
		dr = (struct dir_entry *) bk->b_data;
		cur_inode = dr->inode;
		dr++;
		fa_inode = dr->inode; 	
		while (cur_dr != root_dr) {
			cur_dr = iget(current->pwd->i_dev, fa_inode);
			bk = bread(current->pwd->i_dev,cur_dr->i_zone[0]);
			dr = (struct dir_entry *) bk->b_data;
			ff_inode = (dr+1)->inode;
			k=0;
			while (k < cur_dr->i_size) {
				if(dr->inode == cur_inode) {
					strcpy(tmp, dr->name);
					strcat(tmp, "/");
					strcat(tmp, ans_buf);
					strcpy(ans_buf, tmp);
					break;
				}
				dr++;
				k+=len_dir;
			}
			cur_inode = fa_inode;
			fa_inode = ff_inode;
		}
	brelse(bk);
	}
	strcpy(tmp, "/");
	strcat(tmp, ans_buf);
	strcpy(ans_buf, tmp);

	if(strlen(ans_buf)>size) return NULL;
	int i;
	for(i = 0; i<ans_buf[i]; i++){
		put_fs_byte(ans_buf[i], buf + i);
	}
	return (long)buf;
}

int sys_coushu(){
	return 1;
};
