#!/bin/bash

separator='-'
pushd `dirname $0` > /dev/null
dir=`pwd -P`
popd > /dev/null

source $dir/common_util.sh
err=0

prefix_build=chaos
outbase=$dir/../
create_deb_ver=""
remove_working="false"
log="$0.log"
on_dir=()



mesg=""
die(){
    error_mesg "$1" " exiting... "
    exit 1
}

git_checkout(){
    dir=$1
    check_out_opt=""

    if git fetch; then
	ok_mesg "[$dir] fetching"
    else
	error_mesg "[$dir] cannot fetch"
	return 1
    fi
    if git branch -a |grep "origin/$2";then
	if git branch | grep "$2" ;then
	    check_out_opt="$2"
	else
	    check_out_opt="-t -b $2 origin/$2"
	fi
    else
	if git branch |grep "$2";then
	    check_out_opt="$2"
	else
	    echo "branch $2 not found, do you want create?, empty skip:"
	    read mesg
	    if [ -z "$mesg" ];then
		return 1
	    fi
	    check_out_opt="-t -b $2"

	fi
    fi

    if git checkout $check_out_opt ; then
	ok_mesg "[$dir] checkout $2"
	if git pull ;then
	    ok_mesg "[$dir] synchronize"
	else
	    nok_mesg "[$dir] synchronize"
	    return 1
	fi
    else
	error_mesg "[$dir] checking out $2"
	return 1
    fi

    return 0

}
git_arg=()
git_cmd=""

usage(){
    echo -e "Usage is $0 [-s] [-t <tag name>][ -c <checkout branch> ] [ -p <branch name> ] [-d <directory0>] [-d <directory1>] \n-c <branch name>: check out a given branch name in all subdirs\n-p <branch>:commit and push modifications of a given branch\n-s:retrive the branch status\n-t <tag name>:make an annotated tag to all\n-d <directory>: apply just to the specified directory\n-m <src branch> <dst branch>: merge src into dst branch\n"
}
while getopts t:c:p:hsd:mr:b opt; do
    case $opt in
	r)
	    remote=1
	    ;;
	t)
	    echo -n "provide a tag message:"
	    read mesg
	    git_cmd=t
	    git_arg=$OPTARG
	    ;;
	d)
	    on_dir+=($OPTARG)
	    ;;
	s)
	    git_cmd=s
	    ;;
	c)
	    git_cmd=c
	    git_arg=$OPTARG
	    ;;
	p)
	    git_cmd=p
	    git_arg=$OPTARG
	    ;;
	m)  git_cmd=m

	    ;;

  b)
      git_cmd=b
      git_arg=$OPTARG

      ;;
	h)
	    usage
	    exit 0
	    ;;
    esac

done
shift $(( OPTIND - 1 ))

if [ ${#on_dir[@]} -eq 0 ]; then
    dirs=$(find . -name ".git" -exec grep -sl opensource \{\}/config \;)
    for d in $dirs;do
	dir=$(dirname $d)
	dir=$(dirname $dir)
	on_dir+=($dir)

    done
fi

if [ ${#on_dir[@]} -eq 0 ]; then
    nok_mesg "no git dirs specified/found"
    exit 1
fi

for dir in ${on_dir[@]}; do
    pushd $dir > /dev/null
    case $git_cmd in
	s)
	    git fetch
	    info_mesg "directory " "$dir"
	    git status
	    ;;

	c)
	    git_checkout $dir $git_arg
	    ;;
	m)

	    if [ -z "$1" ] || [ -z "$2" ];then
		echo "## expected source branch and destination branch"
		usage
		exit 1
	    fi
	    echo -n "[$dir] merging branch:\"$1\" in branch:\"$2\", empty comment to skip:"
	    read mesg
	    if [ -n "$mesg" ]; then
		if git_checkout $dir $2; then
		    if git merge -m "$mesg" --no-ff $1;then
			info_mesg "[$dir] merge " "done"
		    else
			error_mesg "[$dir] error merging $1 -> $2, skipping merge"
		    fi
		fi
	    fi
	    ;;
  b)

    	    if [ -z "$1" ] || [ -z "$2" ];then
    		echo "## expected source branch and destination branch"
    		usage
    		exit 1
    	    fi
    	    echo -n "[$dir] rebase branch:\"$1\" against:\"$2\", empty charracter to skip:"
    	    read mesg
    	    if [ -n "$mesg" ]; then
    		if git_checkout $dir $1; then
    		    if git rebase $2;then
    			info_mesg "[$dir] rebase rebasing $1 against $2" "done"
    		    else
    			error_mesg "[$dir] error rebasing $1 against $2, skipping rebase"
    		    fi
    		fi
    	    fi
    	    ;;
	t)
	    git fetch
	    git pull
	    tagname=$(echo "$git_arg" | tr ' ' '-')
	    info_mesg "[$dir] creating tag " "$tagname"
	    git tag -m "$mesg" $tagname
	    git push --tags
	    ;;
	p)
	    if git status | grep modified > /dev/null; then
		echo -n "[$dir] modification found on [branch:$git_arg], empty comment to skip:"
		read mesg
		if [ -n "$mesg" ]; then
			if git commit -m "$mesg" .;then
			    info_mesg "[$dir] commit " "done"
			    if git push origin $git_arg > /dev/null; then
				info_mesg "[$dir] push " "done"
			    else
				error_mesg "[$dir] cannot push"
			    fi
			else
			    error_mesg "[$dir] cannot commit"
			fi

		fi
	    else
		if git status | grep ahead > /dev/null; then
		    if git push > /dev/null; then
			info_mesg "[$dir] push " "done"
		    else
			error_mesg "[$dir] cannot push"
		    fi

		fi
	    fi

	    ;;
    esac

    popd > /dev/null
done
