/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Daniel Ochoa Donoso 2010 <dochoa@fiec.espol.edu.ec>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>

#define SHMSZ     27
int shmidd,shmidt;
key_t keyd,keyt;
char *shmd, *shmt;

int main()
{
	char tmpd[SHMSZ];
    char oldt[SHMSZ];
    char tmpt[SHMSZ];
    keyd = 1234;
    if ((shmidd = shmget(keyd, SHMSZ, 0666)) < 0) {
        perror("shmget");
        return(1);
    }
    if ((shmd = shmat(shmidd, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }
    keyt = 5678;
    if ((shmidt = shmget(keyt, SHMSZ,  0666)) < 0) {
        perror("shmget");
        return(1);
    }
    if ((shmt = shmat(shmidt, NULL, 0)) == (char *) -1) {
        perror("shmat");
        return(1);
    }

    while(1){
		strcpy(tmpt,shmt);
		if ((strcmp(tmpt,"--")!=0)&&(strcmp(oldt,tmpt)!=0)){
			fprintf(stdout,"giroscopio: %s\n",tmpt);
			strcpy(oldt,tmpt);
        }
		if (strcmp(shmd,tmpd)!=0)
			fprintf(stderr,"distancia %s\n",shmd);
		strcpy(tmpd,shmd);

	}
    return(0);

}
