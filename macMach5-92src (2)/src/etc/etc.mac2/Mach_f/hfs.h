#ifndef _HFS_H_
#define _HFS_H_

typedef ParamBlockRec HFSparam;

int HFSopen(HFSparam *, char *);
long HFSread(HFSparam *, char *, long);

#endif /* _HFS_H_ */