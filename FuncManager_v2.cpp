#include "FuncManager_v2.h"

void FuncManager_v2::message(const QString & msg)
{
    log.append(QTime::currentTime().toString("hh:mm:ss:zzz")+" :: "+msg);
}

//Loading all functions from the library
bool FuncManager_v2::loadFunctions(OriginalLibrary * p_library)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(loadFunctions): ");
    message(msg+"Started;");
    QFileInfo fileinfo(p_library->original_name);
    message(msg+"Library ["+fileinfo.baseName()+"] starts to load;");
    p_library->function_count = (*p_library->_functionsCount)();  
    if(!p_library->function_count)
    {
        last_error = msg+"ERROR! Library ["+fileinfo.baseName()+"] has a function count equial to [0];";
        message(last_error);
        return false;
    }
    message(msg+"Library ["+fileinfo.baseName()+"] has ["+QString::number(p_library->function_count)+"] functions;");
    char * tempnames = (*p_library->_functionsNames)();
    if(!tempnames)
    {
        last_error = msg+"ERROR! Library ["+fileinfo.baseName()+"] does not return function declarations;";
        message(last_error);
        return false;
    }
    QString names(tempnames);
    delete [] tempnames;
    QStringList namelist = names.split(" ",QString::SkipEmptyParts);
    if(p_library->function_count!=namelist.size())
    {
        last_error = msg+"ERROR! Library ["+fileinfo.baseName()+"] has ["+
                QString::number(p_library->function_count)+
                "] functions, but sent ["+QString::number(namelist.size())+
                "] names;";
        message(last_error);
        return false;
    }
    message(msg+"Library ["+fileinfo.baseName()+"] send a function declarations;");
    for(int i = 0; i<namelist.size();i++)
    {
        message("/t"+msg+"Function ["+namelist.at(i)+"] is start to load;");
        OriginalFunction * signatures = nullptr;
        signatures = loadFunction(p_library, namelist.at(i));
        if(!signatures)
        {
            last_error=msg+"ERROR! Function ["+namelist.at(i)+"] failed when it was loading;";
            message(last_error);
            return false;
        }
        else
        {
            message("\t"+msg+"Function ["+namelist.at(i)+"] load successfully;");
            p_library->functions.insert(namelist.at(i),signatures);
            contained_functions.insert(namelist.at(i),p_library);
        }
    }
    message(msg+"Finished;");
    return true;

}

//Loading info about direct function from a library
FuncManager_v2::OriginalFunction * FuncManager_v2::loadFunction(OriginalLibrary *p_library,
                                                                const QString &name)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("\t(loadFunction): ");
    message(msg+"Started;");
    QByteArray barr = name.toLocal8Bit();
    char * temp_name = barr.data();
    message(msg+"Call to library for ["+name+"] function interface signature;");
    char * temp_whole_interfaces=(*(p_library->_getFunctionInterface))(temp_name);
    if(!temp_whole_interfaces)
    {
        last_error=msg+"ERROR! Library has no responce for ["+name+"] function interface;";
        message(last_error);
        return nullptr;
    }
    message(msg+"["+name+"] function interface load;");
    OriginalFunction * temp = new OriginalFunction;
    if(local_operators.contains(name))
    {
        message(msg+"["+name+"] function is an operator;");
        temp->is_Operator=true;
    }
    else
    {
        message(msg+"["+name+"] function isn't an operator;");
        temp->is_Operator=false;
    }
    QString whole_interfaces(temp_whole_interfaces);
    delete [] temp_whole_interfaces;
    QStringList temp_signature = std::move(whole_interfaces.split(" ",QString::SkipEmptyParts));
    message(msg+"["+name+"] function has ["+QString::number(temp_signature.size())+"] interfaces;");
    if(!temp_signature.isEmpty())
    {
        message(msg+"["+name+"] function signature is ("+whole_interfaces+");");
        temp->signatures = std::move(temp_signature);
    }
    for(int i = 0; i <temp->signatures.size();i++)
    {
        QString & str = temp->signatures[i];
        if(str==func_nulltype)
        {
            temp->signatures[i].clear();
        }
    }
    int len = name.size();
    temp->f_name=new char[len+1];
    strncpy(temp->f_name,temp_name,len);
    temp->f_name[len]='\0';
    message(msg+"["+name+"] function info was download;");
    message(msg+"Finished;");
    return temp;
}

bool FuncManager_v2::removeFunctions(OriginalLibrary * p_library)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(removeFunctions): ");
    message(msg+"Started;");
    QFileInfo fileinfo(p_library->original_name);
    message(msg+" Functions of ["+fileinfo.baseName()+"] library will clear from the application;");
    QMap<QString,OriginalFunction*>::iterator iter = p_library->functions.begin();
    while(iter!=p_library->functions.end())
    {
        OriginalFunction * temp = iter.value();
        message("\t"+msg+"Function ["+temp->f_name+"] is removing now;");
        temp->signatures.clear();
        if(temp->f_name)
        {
            delete [] temp->f_name;
        }
        contained_functions.remove(iter.key(),p_library);
        delete temp;
        iter.value() = nullptr;
        message("\t"+msg+"The function removed;");
        //removeFunction(iter.key(),p_library);
        iter++;
    }
    message(msg+" ["+fileinfo.baseName()+"] library cleared of functions;");
    p_library->functions.clear();
    message(msg+"Finished;");
    return true;
}

bool FuncManager_v2::removeFunction(const QString & name, OriginalLibrary * p_library)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(removeFunction): ");
    message(msg+"Started;");
    if(p_library)
    {
        QFileInfo fileinfo(p_library->original_name);
        message(msg+"Function name ["+name+"] will removed from ["+
                fileinfo.baseName()+"] library;");
        auto itr = p_library->functions.find(name);
        if(itr==p_library->functions.end())
        {
            last_error = msg+"ERROR! Function ["+name+"] can't be found in ["+fileinfo.baseName()+"] library;";
            message(last_error);
            return false;
        }
        OriginalFunction * temp = itr.value();
        message(msg+"Function name ["+name+"] found;");
        if(temp->f_name)
        {
            delete [] temp->f_name;
        }
        temp->signatures.clear();
        contained_functions.remove(name,p_library);
        delete temp;
        p_library->functions.remove(name);
        message(msg+"Function name ["+name+"] removed from ["+fileinfo.baseName()+"] library;");
    }
    else
    {
        QMap<QString,OriginalLibrary*>::iterator iter = libraries.begin();
        while(iter!=libraries.end())
        {
            OriginalLibrary * temp = iter.value();
            /*QFileInfo fileinfo(temp->original_name);*/
            message(msg+"Function name ["+name+"] will removed from ["+
                    /*fileinfo.baseName()*/iter.key()+"] library if it exists;");
            if(temp->functions.contains(name))
            {
                removeFunction(name,temp);
                message(msg+"Function name ["+name+"] removed from ["+
                        /*fileinfo.baseName()*/iter.key()+"] library;");
            }
            iter++;
        }
        message(msg+"["+QString::number(contained_functions.remove(name))+"] functions was removed;");;
    }
    message(msg+"Finished;");
    return true;
}


FuncManager_v2::FuncManager_v2()
{
    message("Basic function manager ver.2 is creating;");
    QString msg("(FuncManager_v2): ");
    message(msg+"Basic implicit casting order is [int]>>[double]>>[string]>>[bool];");
    operation_order.resize(OPERATOR_COUNT);
    operation_order[0]='v';
    operation_order[1]='i';
    operation_order[2]='d';
    operation_order[3]='s';
    operation_order[4]='b';
    message(msg+"Operators are loading;");
    is_ready = true;
    is_ready&=appendOperator("!",8,1);
    is_ready&=appendOperator("++",7,1,leftAssoc);
    is_ready&=appendOperator("--",7,1,leftAssoc);
    is_ready&=appendOperator("^",6,2,leftAssoc);
    is_ready&=appendOperator("&",5,2,leftAssoc|mathVal);
    is_ready&=appendOperator("|",5,2,leftAssoc|mathVal);
    is_ready&=appendOperator("*",4,2,leftAssoc);
    is_ready&=appendOperator("/",4,2,leftAssoc);
    is_ready&=appendOperator("%",4,2,leftAssoc|mathVal);
    is_ready&=appendOperator("+",3,2,leftAssoc|mathVal);
    is_ready&=appendOperator("-",3,2,leftAssoc|mathVal);
    is_ready&=appendOperator("==",2,2,leftAssoc);
    is_ready&=appendOperator("<",2,2,leftAssoc);
    is_ready&=appendOperator(">",2,2,leftAssoc);
    is_ready&=appendOperator(">=",2,2,leftAssoc);
    is_ready&=appendOperator("<=",2,2,leftAssoc);
    is_ready&=appendOperator("!=",2,2,leftAssoc);
    is_ready&=appendOperator("=",1,2,leftAssoc);
    is_ready&=appendOperator("+=",1,2,leftAssoc);
    is_ready&=appendOperator("-=",1,2,leftAssoc);
    is_ready&=appendOperator("*=",1,2,leftAssoc);
    is_ready&=appendOperator("/=",1,2,leftAssoc);
    if(!is_ready)
    {
        last_error = msg+"ERROR! Operators wasn't load;";
        message(last_error);
    }
    else
    {
        message(msg+"Basic function library is loading;");
#ifdef WIN32
        is_ready&=loadNewLibrary("lib/basic_autotest_functions.dll");
#else
        is_ready&=loadNewLibrary("lib/basic_autotest_functions.so");
#endif
        if(!is_ready)
        {
            last_error = msg+"ERROR! Basic function library wasn't load;";
            message(last_error);
        }
        else
        {
            message(msg+"User's operators are loading;");
            is_ready&=loadAdittionalOperators();
            if(!is_ready)
            {
                last_error = msg+"ERROR! User's additional operators can't load;";
                message(last_error);
            }
            else
            {
                message(msg+"Is ready to work;");
            }
        }
    }
}

FuncManager_v2::~FuncManager_v2()
{
    QMap<QString,OriginalLibrary*>::iterator next,it = libraries.begin();
    while(it!=libraries.end())
    {
        next=it;
        next++;
        freeLibrary(it.key());
        it=next;
    }
}

bool FuncManager_v2::contains(const QString & name)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    return contained_functions.contains(name);
}

bool FuncManager_v2::isOperator(const QString & name)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    return local_operators.contains(name);
}

bool FuncManager_v2::loadAdittionalOperators()
{
    return true;
}

bool FuncManager_v2::loadNewLibrary(const QString & name)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(loadNewLibrary): ");
    message(msg+"Started;");
    QFileInfo fileinfo(name);
    message(msg+"Library with name ["+fileinfo.baseName()+"] start to load;");
    message(msg+"Full path ["+fileinfo.absoluteFilePath()+"];");
    if(libraries.contains(fileinfo.baseName()))//if(libraries.contains(name))
    {
        last_error=msg+"ERROR! Library with name ["+fileinfo.baseName()+"] already load;";
        message(last_error);
        return false;
    }
    if(!fileinfo.exists())
    {
        last_error=msg+"ERROR! Library with name ["+fileinfo.baseName()+"] doesn't exist in specified directory;";
        message(last_error);
        message(msg+"Specified directory ["+fileinfo.filePath()+"];");
        return false;
    }

    OriginalLibrary * temp = new OriginalLibrary;
    temp->_callFunction=nullptr;
    temp->_functionsCount=nullptr;
    temp->_functionsNames=nullptr;
    temp->_getFunctionInterface=nullptr;
    temp->_initiateDLL=nullptr;
    temp->_releaseDll=nullptr;

    message(msg+"Trying to establish an interface with ["+fileinfo.baseName()+"] library;");
    QString canonpath = fileinfo.canonicalFilePath();
    temp->library.setFileName(/*name*/canonpath);
    if(temp->library.load())
    {
        message(msg+"["+fileinfo.baseName()+"] library responds to the manager;");
        message(msg+"Library is establishing connections with manager functions;");
        temp->original_name=name;
        temp->_getFunctionInterface =(GET_INTRFCE)temp->library.resolve("_atf_getFuncInterfaces");
        temp->_releaseDll =(RELEASE_DLL)temp->library.resolve("_atf_freeDLL");
        temp->_initiateDLL = (INIT_DLL)temp->library.resolve("_atf_initiateDLL");
        temp->_functionsCount=(FUNC_COUNT)temp->library.resolve("_atf_functionCount");
        temp->_functionsNames=(FUNC_NAMES)temp->library.resolve("_atf_functionNames");
        temp->_callFunction = (CALL_FUNC)temp->library.resolve("_atf_callFunction");
        if(     (temp->_getFunctionInterface)&&
                (temp->_callFunction)&&
                (temp->_functionsCount)&&
                (temp->_functionsNames)&&
                (temp->_initiateDLL)&&
                (temp->_releaseDll))
        {
            message(msg+"Connections esteblished successfuly;");
            (*temp->_initiateDLL)();
        }
        else
        {
            last_error=msg+"ERROR! Connection establishment failed;";
            message(last_error);
            delete temp;
            return false;
        }
        if(loadFunctions(temp))
        {
            message(msg+"Library ["+fileinfo.baseName()+"] appends to the libraries map;");
            libraries.insert(fileinfo.baseName()/*name*/,temp);
            return true;
        }
        else
        {
            last_error=msg+"ERROR! Manager doesn't load functions from ["+fileinfo.baseName()+"] library;";
            removeFunctions(temp);
            temp->library.unload();
            delete temp;
            return false;
        }
    }

    last_error=msg+"ERROR! Library doesn't load with isslue ["+temp->library.errorString()+"];";
    delete temp;
    return false;
}

bool FuncManager_v2::freeLibrary(const QString & name)
{

    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(freeLibrary): ");
    message(msg+"Started;");
    QFileInfo fileinfo(name);
    if(!libraries.contains(name))
    {
        last_error=msg+"ERROR! Manager doesn't find library  with name ["+fileinfo.baseName()+"];";
        return false;
    }
    OriginalLibrary * temp = libraries.take(name);
    message(msg+"["+fileinfo.baseName()+"] library removed from the library map;");
    (*temp->_releaseDll)();
    message(msg+"["+fileinfo.baseName()+"] library interface released;");
    if(temp->library.unload())
    {
        message(msg+"["+fileinfo.baseName()+"] library unloaded from the memory;");
    }
    else
    {
        message(msg+"WARNING! ["+fileinfo.baseName()+"] library doesn't unload from the memory;");
    }
    if(removeFunctions(temp))
    {
        message(msg+"["+fileinfo.baseName()+"] library functions erased from the memory;");
    }
    else
    {
        message(msg+"WARNING! ["+fileinfo.baseName()+"] library functions don't erase from the memory;");
    }
    delete temp;
    last_error.clear();
    message(msg+"Finished;");
    return true;
}

bool FuncManager_v2::appendOperator(const QString  & name,
                                    int priority, int arg_count,
                                    char flags)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(appendOperator): ");
    message(msg+"Started;");
    message(msg+"Manager tries to append new ["+name+"] operator;");
    if(local_operators.contains(name))
    {
        last_error=msg+"ERROR! Operator ["+name+"] already exists;";
        message(last_error);
        return false;
    }
    OriginalOperator * temp = new OriginalOperator;
    temp->flags=flags;
    temp->priority=priority;
    temp->operand_count=arg_count;
    local_operators.insert(name,temp);
    message(msg+"New operator ["+name+"] append;");
    message(msg+"Finished;");
    return true;
}

bool FuncManager_v2::removeOperator(const QString & name)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(removeOperator): ");
    message(msg+"Started;");
       message(msg+"Manager tries to remove ["+name+"] operator;");
    if(local_operators.contains(name))
    {
        OriginalOperator * temp = local_operators.take(name);
        delete temp;
        message(msg+"Operator ["+name+"] remove;");
        message(msg+"Finished;");
        return true;
    }
    last_error="ERROR! ["+name+"] Operator doesn't exist;";
    message(last_error);
    return false;
}

data_ptr FuncManager_v2::callFunction(const std::shared_ptr<FuncObject_v2> &function, QVector<var_ptr> & data)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return nullptr;
    }
    QString msg("(callFunction): ");
    message(msg+"Started;");
    QString typestring, func_name=function->fName();
    message(msg+"Manager starts to execute function ["+func_name+"];");
    message(msg+"Variables count ["+QString::number(data.size())+"];");
    if(data.size())
    {
        typestring.reserve(data.size());
        foreach (var_ptr variable, data)
        {
            typestring+=variable->getCtype();
        }
    }
    message(msg+"Manager is trying to find ["+func_name+"] function;");
    QMultiMap<QString, OriginalLibrary*>::iterator approp_lib = contained_functions.find(func_name);

    if(approp_lib==contained_functions.end())
    {
        last_error=msg+"ERROR! Manager can't find function ["+func_name+"] in the library map;";
        message(last_error);
        return nullptr;
    }
    message(msg+"Manager find at least one function with name ["+func_name+"];");
    local_foundry option_for_call;
    QChar various_ch('v');
    option_for_call.best_lib=nullptr;
    option_for_call.best_count=-1;
    option_for_call.function_is_found = false;
    message(msg+"Manager tries to find the most optimal interface with ["+typestring+"] signature;");
    while(approp_lib.key()==func_name)
    {
        OriginalLibrary * current_lib = approp_lib.value();
        message("\t"+msg+"Current lib ["+current_lib->original_name+"];");
        if(!function->fPref().isEmpty())
        {
            if(current_lib->library.fileName()!=function->fPref())
            {
                approp_lib++;
                continue;
            }
        }
        OriginalFunction * current_function = current_lib->functions.find(func_name).value();
        int count_x = current_function->signatures.size();
        for(int i = 0; i<count_x;i++)
        {

            QString allow_signature = current_function->signatures.at(i);
            message("\t\t"+msg+"Current signature ["+allow_signature+"];");
            //allow_signature.remove(0,1);
            if((typestring.isEmpty()/*&&typestring.isEmpty()*/)||
                             (typestring==allow_signature))
            {
                option_for_call.best_count = 0;
                option_for_call.function_is_found = true;
                option_for_call.best_lib = current_lib;
                option_for_call.best_signature=allow_signature;
                break;
            }
            else if(typestring.size()==allow_signature.size())
            {
                int optional_count = 0;
                for(int j=0;j<typestring.size();j++)
                {
                    if((typestring.at(j)==allow_signature.at(j))||(allow_signature.at(j)==various_ch))
                    {
                        optional_count++;
                    }
                }
                if(option_for_call.best_count<optional_count)
                {
                    option_for_call.best_lib=current_lib;
                    option_for_call.best_count=optional_count;
                    option_for_call.best_signature = allow_signature;
                }
                else if(option_for_call.best_count==optional_count)
                {
                    int ble = allow_signature.size();
                    int lopt=0; QByteArray lbyte = option_for_call.best_signature.toLatin1();
                    int ropt=0; QByteArray rbyte = allow_signature.toLatin1();
                    for(int i = 0; i<ble;i++)
                    {
                           char res = op_get_greater(lbyte.at(i),rbyte.at(i));
                           if(res==lbyte.at(i))
                           {
                               lopt++;
                           }
                           else
                           {
                               ropt++;
                           }
                    }
                    if(ropt>lopt)
                    {
                        option_for_call.best_lib=current_lib;
                        option_for_call.best_signature = allow_signature;
                    }
                }
            }
        }
        if(option_for_call.function_is_found)
        {
            break;
        }
        else
        {
            approp_lib++;
        }
    }

    if(option_for_call.best_count<0)
    {
        last_error=msg+"ERROR! Function ["+func_name+"] with signature ["+typestring+"] can't be find;";
        message(msg+last_error);
        return nullptr;
    }
    message(msg+"Function ["+func_name+"] with appropriative interface ["+option_for_call.best_signature+"] was found;");
    message(msg+"Data cast is starting;");
    char ** ii_responce = data_comp(func_name,data,option_for_call);
    if(!ii_responce)
    {
        last_error=msg+"ERROR! Function ["+func_name+"] isn't responding after dynamic call;";
        message(last_error);
        return nullptr;
    }
    message(msg+"Returned message analyse is starting;");
    data_ptr ret_val = ret_analyse(ii_responce/*,func_name*/);

    delete [] ii_responce[0];
    delete [] ii_responce[1];
    delete [] ii_responce;
    message(msg+"Finished;");
    return ret_val;

}

char ** FuncManager_v2::data_comp(const QString &func_name, const QVector<var_ptr> &data, local_foundry & selected_lib)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return nullptr;
    }
    QString msg("(data_comp): ");
    message(msg+"Started;");
    message(msg+"Manager starts to run function ["+func_name+"];");
    message(msg+"Data size is ["+QString::number(data.size())+"];");
    int realsize = data.size()*2;
    char ** result = nullptr;
    char ** sendmail=nullptr;
    if(realsize)
    {
       sendmail = new char *[realsize];
    }

    message(msg+"Container for variables was created;");

    QList<char*> temp_data;
    char cInt[] ={'i','\0'};// new char[2];
    char cDob[] ={'d','\0'};// new char[2];
    char cStr[] ={'s','\0'};// new char[2];
    char cBool[]={'b','\0'};// new char[2];
    char cVar[] ={'v','\0'};
    //cInt[1]=cDob[1]=cStr[1]=cBool[1]='\0';
    //cInt[0]='i';cDob[0]='d';cStr[0]='s';cBool[0]='b';
    //char cInt[] {"i\0"}, cDob[]{"d\0"}, cStr[]{"s\0"}, cBool[]{"b\0"};
    for(int i = 0,j=0; i<data.size();i++,j+=2)
    {
        switch(selected_lib.best_signature.at(i).toLatin1())
        {
        case 'i':
        {
            message("\t"+msg+"Type of the ["+QString::number(i)+"] variable is integer;");
            sendmail[j]=cInt;
        }
            break;
        case 'd':
        {
            message("\t"+msg+"Type of the ["+QString::number(i)+"] variable is double;");
            sendmail[j]=cDob;
        }
            break;
        case 's':
        {
            message("\t"+msg+"Type of the ["+QString::number(i)+"] variable is string;");
            sendmail[j]=cStr;
        }
            break;
        case 'b':
        {
            message("\t"+msg+"Type of the ["+QString::number(i)+"] variable is bool;");
            sendmail[j]=cBool;
        }
            break;
        case 'v':
        {
            message("\t"+msg+"Type of the ["+QString::number(i)+"] variable is variant;");
            sendmail[j]=cVar;
        }
            break;
        default:
        {
               qDeleteAll(temp_data);
               delete [] sendmail;
               last_error=msg+"ERROR! The type of the variable N["+QString::number(i)+"] is uncorrect;";
               message("\t"+last_error);
               return nullptr;
        }
            break;
        }

        if((selected_lib.best_signature.at(i)==data.at(i)->getCtype())||(selected_lib.best_signature.at(i)=='v'))
        {
            message("\t"+msg+"The type of the varible N["+QString::number(i)+"] matches the sample;");
            sendmail[j+1]=data.at(i)->getData();
            temp_data.append(sendmail[j+1]);
        }
        else
        {
            message("\t"+msg+"The type of the varible N["+QString::number(i)+"] doesn't match the sample;");
            VarObject_v2 temp;
            var_ptr backtemp = data.at(i);
                    //std::make_shared<VarObject_v2>(*(data.at(i).get()));
            message("\t"+msg+"Manager is trying to cast a variable to the needed type;");
            switch(selected_lib.best_signature.at(i).toLatin1())
            {
            case 'i':
            {
                message("\t"+msg+"Manager is trying to cast a variable to integer type;");
                VarObject_v2 * var = backtemp.get();
                temp = static_cast<int>(*var);
            }
                break;
            case 'd':
            {
                message("\t"+msg+"Manager is trying to cast a variable to double type;");
                VarObject_v2 * var = backtemp.get();
                temp = static_cast<double>(*var);
            }
                break;
            case 's':
            {
                message("\t"+msg+"Manager is trying to cast a variable to string type;");
                char * ctemp = backtemp->getData();
                temp = ctemp;
                delete [] ctemp;
            }
                break;
            case 'b':
            {
                message("\t"+msg+"Manager is trying to cast a variable to bool type;");
                VarObject_v2 * var = backtemp.get();
                temp = static_cast<bool>(*var);
            }
                break;
            case 'v':
            {
                message("\t"+msg+"The type of this variable does not matter for the result;");
                temp = backtemp.get();
            }
                break;
            }

            sendmail[j+1]=temp.getData();
            temp_data.append(sendmail[j+1]);
            message("\t"+msg+"Variable was add to the container;");
        }
    }
    try
    {
        result = (*selected_lib.best_lib->_callFunction)(func_name.toLatin1().data(),/*realsize*/ data.size(),sendmail);
    } catch(const std::exception & err)
    {
        QString error(err.what());
        message(msg+"CRITICAL ERROR! Unexpected plugin behavior. Type ["+error+"];");
        if(result)
        {
            delete [] result;
        }
        result = nullptr;
    }

    qDeleteAll(temp_data);
    delete [] sendmail; //delete [] cInt; delete [] cDob; delete [] cStr; delete []cBool;
    message(msg+"Finished;");
    return result;
}

data_ptr FuncManager_v2::ret_analyse(char ** ii_responce/*, const QString & func_name*/)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return nullptr;
    }
    QString msg("(ret_analyse): ");
    message(msg+"Started;");
    data_ptr return_val;
    QString string;
    message(msg+"Manager starts to analyse the answer from the library;");
    char type = ii_responce[0][0]; //get the type of the return data
    char * dat_t = ii_responce[1];
    switch(type)
    {
    case 'e':
        message(msg+"Return data type is an error message;");
    case 'c':
    {
        message(msg+"Return data type is a command;");
        signal_ptr temp = std::make_shared<SignalObject_v2>(dat_t);
        return_val = std::static_pointer_cast<DataObject>(temp);
    }
        break;
    case 'i':
    {
        message(msg+"Return data type is integer;");
        string=ii_responce[1];
        return_val = data_ptr(new VarObject_v2(string.toInt()));
    }
        break;
    case 'd':
    {
        message(msg+"Return data type is double;");
        string=ii_responce[1];
        return_val = data_ptr(new VarObject_v2(string.toDouble()));
    }
        break;
    case's':
    {
        message(msg+"Return data type is a string;");
        return_val = data_ptr(new VarObject_v2(ii_responce[1]));
    }
        break;
    case 'b':
    {
        message(msg+"Return data type is bool;");
        string=ii_responce[1];
        return_val = data_ptr(new VarObject_v2((string=="true")||(string=="1")));
    }
        break;
    default:
    {
        last_error=msg+"ERROR! Return data type is unknown ["+type+"];";
        message(last_error);
        return nullptr;
    }
        break;
    }
    message(msg+"Finished;");
    return return_val;
}

QString FuncManager_v2::getError() const
{
    return last_error;
}

QStringList FuncManager_v2::readLog()
{
    message("__Current Function Manager log is end__;");
    QStringList temp(std::move(log));
    log.clear();
    return temp;
}

bool FuncManager_v2::changeTypePriority(char lesser, char less, char great, char greater)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return false;
    }
    QString msg("(changeTypePriority): ");
    message(msg+"Started;");
    message(msg+"New type priority is ["+lesser+"]>>["+less+"]>>["+great+"]>>["+greater+"];");
    operation_order.clear();
    operation_order.resize(4);
    operation_order[1]=lesser;
    operation_order[2]=less;
    operation_order[3]=great;
    operation_order[4]=greater;
    message(msg+"Finished;");
    return true;
}

char FuncManager_v2::op_get_greater(char left, char right) const
{
    int i = operation_order.indexOf(left);
    if(i<0)
    {
        return 0;
    }
    if(i<operation_order.indexOf(right))
    {
        return right;
    }
    return left;
}

int FuncManager_v2::op_preced(const QString & op)
{
    if(local_operators.contains(op))
    {
       OriginalOperator * temp = local_operators.find(op).value();
       return temp->priority;
    }
    return 0;
}

bool FuncManager_v2::op_left_assoc(const QString & op)
{
    if(local_operators.contains(op))
    {
       OriginalOperator * temp = local_operators.find(op).value();
       return ((temp->flags)&leftAssoc);
    }
    return false;
}

bool FuncManager_v2::op_is_math(const QString &op)
{
    if(local_operators.contains(op))
    {
       OriginalOperator * temp = local_operators.find(op).value();
       return ((temp->flags)&mathVal);
    }
    return false;
}

int FuncManager_v2::op_arg_count(const QString &op)
{
    if(local_operators.contains(op))
    {
       OriginalOperator * temp = local_operators.find(op).value();
       return temp->operand_count;
    }
    return 0;
}

bool FuncManager_v2::isLibLoad(const QString &libname)
{
    return libraries.contains(libname);
}

QString FuncManager_v2::getFunctionInterface(const QString &libname, const QString & name)
{
    QString interface;
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return interface;
    }

    QString msg("(getFunctionInterface): ");
    message(msg+"Started;");
    message(msg+"Manager is searching for library ["+libname+"];");
    QMap<QString,OriginalLibrary*>::iterator libs = libraries.find(libname);
    if(libs!=libraries.end())
    {
        message(msg+"Sought library is found;");
        OriginalLibrary * cur_lib = libs.value();
        message(msg+"Manager is searching for function["+name+"];");
        QMap<QString,OriginalFunction*>::iterator funcs = cur_lib->functions.find(name);
        if(funcs!=cur_lib->functions.end())
        {
            message(msg+"Sought function is found;");
            OriginalFunction * cur_func = funcs.value();
            interface.append(cur_func->signatures.join(" "));
        }
        else
        {
            message(msg+"WARNING! Sought function isn't found;");
        }
    }
    else
    {
        message(msg+"WARNING! Sought library isn't found;");
    }
    message(msg+"Finished;");
    return interface;
}

QList<QStringList> FuncManager_v2::getAllInterfaces()
{
    QList<QStringList> all_interfaces;
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return all_interfaces;
    }
    QString msg("(getAllInterfaces): ");
    message(msg+"Started;");
    message(msg+"Manager is collecting all libraries and functions into a list;");
    QMap<QString,OriginalLibrary*>::iterator libs = libraries.begin();
    while(libs!=libraries.end())
    {
        OriginalLibrary * cur_lib = libs.value();
        message("\t"+msg+"Current library is ["+cur_lib->original_name+"];");
        QMap<QString,OriginalFunction*>::iterator funcs = cur_lib->functions.begin();
        while(funcs!=cur_lib->functions.end())
        {
            OriginalFunction * cur_func = funcs.value();
            message("\t\t"+msg+"Current function is ["+cur_func->f_name+"];");
            QStringList single_func;
            single_func.append(libs.key());
            single_func.append(funcs.key());
            single_func<<cur_func->signatures;
            message("\t\t"+msg+"Function signatures are ["+cur_func->signatures.join("; ")+"];");
            all_interfaces.append(single_func);
            funcs++;
        }
        libs++;
    }
    message(msg+"Finished;");
    return all_interfaces;
}

QVariant FuncManager_v2::manualFuncCall(const QString & libname,
                                        const QString & name,
                                        const QVector<QVariant> & data)
{
    if(!is_ready)
    {
        message("ERROR! Function maganer created with issues;");
        return QVariant();
    }
    QString msg("(manualFuncCall): ");
    message(msg+"Started;");
    message(msg+"The manager is trying to find the library ["+libname+"];");
    lib_map::iterator libs = libraries.find(libname);
    local_foundry call_options;
    if(libs!=libraries.end())
    {
        message(msg+"Library is found;");
        call_options.best_lib = libs.value();
        message(msg+"Manager tries to find function ["+name+"];");
        if(call_options.best_lib->functions.contains(name))
        {
            call_options.function_is_found=true;
            message(msg+"Function is found;");
            message(msg+"The manager is trying to prepare the data for the function;");
            QString signature;
            QVector<var_ptr> variables;
            for(int i = 0;i<data.size();i++)
            {
                switch(data.at(i).type())
                {
                case QVariant::Int:
                {
                    signature+='i';
                    variables.append(std::make_shared<VarObject_v2>(data.at(i).toInt()));
                }
                    break;
                case QVariant::Double:
                {
                    signature+='d';
                    variables.append(std::make_shared<VarObject_v2>(data.at(i).toDouble()));
                }
                    break;
                case QVariant::Bool:
                {
                    signature+='b';
                    variables.append(std::make_shared<VarObject_v2>(data.at(i).toBool()));
                }
                    break;
                case QVariant::String:
                {
                    signature+='s';
                    variables.append(std::make_shared<VarObject_v2>(data.at(i).toString().toLatin1().data()));
                }
                    break;
                default:
                {
                    last_error = msg+"ERROR! The ["+QString::number(i+1)+
                            "]th variable type is ["+data.at(i).typeName()+
                            "] can't be processed;";
                    message(last_error);
                    return QVariant();
                }
                }
            }
            call_options.best_signature=signature;
            call_options.best_count=data.size();
            char ** ii_responce = data_comp(name,variables,call_options);
            if(!ii_responce)
            {
                last_error=msg+"ERROR! Function ["+name+"] doesn't responding after dynamic call;";
                message(last_error);
                return QVariant();
            }
            data_ptr data_answer = ret_analyse(ii_responce/*,func_name*/);
            delete [] ii_responce[0];
            delete [] ii_responce[1];
            delete [] ii_responce;
            message(msg+"Finished;");
            return ret_manual(data_answer);
        }
        else
        {
            last_error= msg+"ERROR! The function ["+name+"] can't be found;";
            message(last_error);
            return QVariant();
        }
    }
    else
    {
        last_error = msg+"ERROR! Library ["+libname+"] can't be found;";
        message(last_error);
        return QVariant();
    }
    return QVariant();
}

QVariant FuncManager_v2::ret_manual(data_ptr value)
{
    QVariant answer;
    switch(value->objType())
    {
    case Tvariable:
    {
        var_ptr tempvar = std::dynamic_pointer_cast<VarObject_v2>(value);
        switch(tempvar->getType())
        {
        case Type::Tint:
        {
            answer=static_cast<int>(*tempvar.get());
        }
            break;
        case Type::Tdouble:
        {
            answer=static_cast<double>(*tempvar.get());
        }
            break;
        case Type::Tbool:
        {
            answer=static_cast<bool>(*tempvar.get());
        }
            break;
        case Type::Tstring:
        {
            char * temp_str = static_cast<char*>(*tempvar.get());
            answer = QString(temp_str);
            delete [] temp_str;
        }
            break;
        default:
            break;
        }
    }
        break;
    case Tsignal:
    {
        signal_ptr tempsig = std::dynamic_pointer_cast<SignalObject_v2>(value);
        int sign_type = tempsig->getType();
        answer = QString("Signal:")+tempsig->signalName(sign_type)+";";
    }
        break;
    default:
        break;
    }
    return answer;
}
