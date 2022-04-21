#include "ExcelApp.h"


///----------------------MESSAGE FUNCTIONS--------------------------------
void ExcelApp::__message(const char * message, int status)
{

    __message(QString(message),status);
}

void ExcelApp::__message(const QString & message, int status)
{
    QString logmsg = QTime::currentTime().toString("HH:mm:ss:zzz");
    logmsg+="::"+message;
    //log.append(logmsg);
     __writelog(logmsg);
    if(status==scr_INFO)
    {
        if(!(userflags&ExApp_ShowInfoMsg))
        {
            return;
        }
    }
    else if(status==scr_WARNING)
    {
        if(!(userflags&ExApp_ShowWarnMsg))
        {
            return;
        }
    }
    emit Excel_signal_message(logmsg,status);
}

///----------------------CONSTRUCTORS-------------------------------------
ExcelApp::ExcelApp(QObject *parent) : /*QWidget*/QObject(parent)
{
    __Excel_slot_start_clear();
    __create_current_log_file();
    Excel_slot_clear();

}

ExcelApp::ExcelApp(const QString & excAddr,
         const QString & templName,
         const QString & sheetName,
         const QString & confDir,
         const QString & scrDir, char flag,
         QObject *parent):/*QWidget*/QObject(parent)
{
    __Excel_slot_start_clear();
    __create_current_log_file();
        Excel_reupload_data(excAddr,templName,
                             sheetName,confDir,
                             scrDir,flag);
}

ExcelApp::~ExcelApp()
{

    Excel_slot_clear();
    __writelog("ExcelApp working log end;");
}
/*
void ExcelApp::__writelog()
{
    QDir dir(QDir::currentPath());
    dir.mkdir(QDir::currentPath()+"/log");
    QString datename(QDateTime::currentDateTime().toString("dd.MM.yyyy_hh.mm"));
    QFile file(QDir::currentPath()+"/log/log_"+datename+".log");
    if(file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream(&file);
        QStringList::iterator it = log.begin();
        while(it!=log.end())
        {
            stream<<*it<<endl;
            it++;
        }
        file.close();
    }

}
*/

void ExcelApp::__create_current_log_file()
{ 
    QDir dir(QDir::currentPath());
    QString curpath(QDir::currentPath()+"/"+EXCEL_LOG_ADDR+"/");
    dir.mkdir(curpath);
    QString datename(QDateTime::currentDateTime().toString("dd.MM.yyyy_hh.mm"));
    _log_file_n = curpath+datename+"_EXCEL.log";
    QFile file(_log_file_n);
    if(file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QTextStream stream(&file);
        stream<<"ExcelApp working log started;"<<endl;
        file.close();
    }
}

void ExcelApp::__writelog(const QString & text)
{
    if(!_log_file_n.isEmpty())
    {
        QFile file(_log_file_n);
        if(file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
        {
            QTextStream stream(&file);
            stream<<text<<endl;
            file.close();
        }
    }

}

///----------------------EXCEL INITIATION----------------------------------
void  ExcelApp::Excel_reupload_data(const QString & excAddr,
                         const QString & templName,
                         const QString & sheetName,
                         const QString & confDir,
                         const QString & scrDir,
                         char flag)
{
    __message("Procedure [Excel_reupload_data] started;", MSG_INFO);
    QString msg("(Excel_reupload_data):");
    Excel_slot_clear();
    char prevflags = userflags;
    userflags = flag;
    userflags|=(prevflags&ExApp_ShowInfoMsg)|(prevflags&ExApp_ShowWarnMsg);
    if(__Excel_slot_initiate (excAddr,templName,
                              sheetName,confDir,
                              scrDir))
    {
        __message(msg+"Excel data successfully initiated;",MSG_INFO);
        if(flag&ExApp_QuickStart)
        {
            __message(msg+"Emmiting signal for quick start;",MSG_INFO);
            emit Excel_signal_state(EF_DataLoaded);
        }
    }
    else
    {
        __message(msg+"!ERROR Excel data initiated with isslues;",MSG_ERROR);
    }
}

void ExcelApp::Excel_slot_set_flags(char flags)
{
    userflags|=flags;
}

void ExcelApp::Excel_slot_remove_flags(char flags)
{
    userflags&=~flags;
}

void ExcelApp::Excel_reupload_seq( QList<QPair<int,int>> * incl_seq,
                         QList<QPair<int,int>> * excl_seq,
                         QList<int> * pauses,
                         int repeats)
{
    __message("Procedure [Excel_reupload_seq] started;", MSG_INFO);
    QString msg("(Excel_reupload_seq):");
    for(;;)
    {
        if(!incl_seq)
        {
            __message(msg+"!ERROR Include sequance list is empty;", MSG_ERROR);
            break;
        }
        __message(msg+"Loading seqences;",MSG_INFO);
        QList<QPair<int,int>>::iterator it = incl_seq->begin();
        while(it!=incl_seq->end())
        {
            QPair<int,int> temp = *it;
            Excel_slot_append_test_seq_range(temp.first, temp.second);
            it++;
        }
        if(_sequance._tests.isEmpty())
        {
            __message(msg+"!ERROR sequances can't be fill';", MSG_ERROR);
            break;
        }
        __message(msg+"Sequences is load",MSG_INFO);
        if(excl_seq)
        {
            __message(msg+"Starting to exclude sequances;",MSG_INFO);
            it = excl_seq->begin();
            while(it!=excl_seq->end())
            {
                QPair<int,int> temp = *it;
                Excel_slot_remove_test_seq_range(temp.first,temp.second);
                it++;
            }
        }
        if(pauses)
        {
            __message(msg+"Loading data pauses;",MSG_INFO);
            QList<int>::iterator jt = pauses->begin();
            while(jt!=pauses->end())
            {
                Excel_slot_append_debug_pause(*jt);
                jt++;
            }
        }
        if(repeats>0)
        {
            __message(msg+"Counting repeats;",MSG_INFO);
            Excel_slot_set_repeat_number(repeats);
        }
        if(userflags&ExApp_QuickStart)
        {
            __message(msg+"Emmiting signal for quick start;",MSG_INFO);
            emit Excel_signal_state(EF_SeqLoaded);
        }
    break;
    }
    if(incl_seq)
    {
        delete incl_seq;
    }
    if(excl_seq)
    {
        delete excl_seq;
    }
    if(pauses)
    {
        delete pauses;
    }

    __message("Procedure [Excel_reupload_seq] finished;", MSG_INFO);
}

bool ExcelApp::__Excel_slot_initiate (  const QString & excAddr,
                            const QString & templName,
                            const QString & sheetName,
                            const QString & confDir,
                            const QString & scrDir)
{

    __message("Procedure [Excel_slot_initiate] started;", MSG_INFO);
    QString msg("(Excel_slot_initiate):");
    if(excAddr.isEmpty()||(!QFile::exists(excAddr)))
    {
        __message(msg+"!ERROR Missing address;", MSG_ERROR);
        return false;
    }
    if(templName.isEmpty())
    {
        __message(msg+"!ERROR Missing template sheet name;", MSG_ERROR);
        return false;
    }

    if(!__Excel_slot_Excel_load(excAddr,templName))
    {
        __message(msg+"!ERROR Excel loading was interrupted;", MSG_ERROR);
        __Excel_slot_Excel_clear();
        return false;
    }
    __message(msg+"Excel successfully initialized. ", MSG_INFO);
    _conf_dir = confDir.isEmpty()? QDir::currentPath()+"/hex/":confDir;
    QDir conf_dir(_conf_dir);
    if(conf_dir.exists())
    {
        __message(msg+"Config directory set as["+
                  _conf_dir+"];", MSG_INFO);
    }
    else
    {
        __message(msg+"!ERROR Config directory ["+
                  _conf_dir+"] not exists;", MSG_ERROR);
        __Excel_slot_Excel_clear();
        return false;
    }

    _script_dir = scrDir.isEmpty()? QDir::currentPath()+"/tests/":scrDir;
    QDir scr_dir(_script_dir);
    if(scr_dir.exists())
    {
        __message(msg+"Script directory set as["+
                  _script_dir+"];", MSG_INFO);
    }
    else
    {
        __message(msg+"!ERROR Config directory ["+
                  _conf_dir+"] not exists;", MSG_ERROR);
        __Excel_slot_Excel_clear();
        return false;
    }

    Excel_slot_set_worksheet(sheetName);
    if(!(flags&EF_SheetLoaded))
    {
        __message(msg+"!ERROR Excel sheet list didn't load;", MSG_ERROR);
        __Excel_slot_Excel_clear();
        return false;
    }
    __message("Procedure [Excel_slot_initiate] finished successfully; ", MSG_INFO);
        return true;
}

void ExcelApp::Excel_slot_set_worksheet(const QString & sheetName)
{
    __message("Excel_slot_set_worksheet started; ", MSG_INFO);
    QString msg("\t(Excel_slot_set_worksheet):");
    bool aok = true;
    for(;;)
    {
        if(!(flags&EF_ExcelLoaded))
        {
            __message(msg+" Excel not load; ", MSG_ERROR);
            aok=false;
            break;
        }

        if(AX.active_worksheet)
        {
            __disconnect_fromCatch(AX.active_worksheet);
            delete AX.active_worksheet;
            AX.active_worksheet = nullptr;
            flags&=~EF_SheetLoaded;
        }

        if(!sheetName.isEmpty())
        {
            AX.active_worksheet = AX.worksheets->querySubObject("Item(const QString&)",sheetName);
            if(AX.active_worksheet)
            {
                __message(msg+"Worksheet with name ["+sheetName+"] was load;' " ,scr_INFO);
                __connect_toCatch(AX.active_worksheet);
                break;
            }
        }

        QString listname;
        QList<QVariant> lists;
        lists<<QVariant();
        lists<<AX.template_worksheet->asVariant();
        AX.template_worksheet->dynamicCall("Copy(const QVariant&,const QVariant&)",lists);
        listname = AX.template_worksheet->property("Name").toString();
        listname.append(" (2)");
        AX.active_worksheet = AX.worksheets->querySubObject("Item(const QString&)",listname);
        if(!AX.active_worksheet)
        {
            __message(msg+" Can't open created worksheet named ["+listname+"]; " ,scr_ERROR);
            aok=false;
            break;
        }
        __connect_toCatch(AX.active_worksheet);
        if(sheetName.isEmpty())
        {
            listname=QDateTime::currentDateTime().toString("dd_MM_hh_mm");
        }
        else
        {
            listname = sheetName;
        }
        AX.active_worksheet->setProperty("Name",QVariant(listname));
        __message(msg+"New worksheet with name ["+listname+"] was created; " ,scr_INFO);
        break;
    }
    if(aok)
    {
        if(!(userflags&ExApp_NotShowExcWndw))
        {
            AX.mainExcel->setProperty("Visible",true);
            AX.active_worksheet->dynamicCall("Activate");
        }
        flags|=EF_SheetLoaded;

        __message(msg+"Start to loading data from selected sheet",MSG_INFO);
        if(__Excel_slot_Data_load())
        {
            __message("Excel_slot_set_worksheet finished successfully;", MSG_INFO);
            return;
        }
        else
        {
            __message(msg+"ERROR! Excel data doesn't load;", MSG_ERROR);
        }

    }
    else
    {
        __message("ERROR! Excel_slot_set_worksheet finished with isslues;", MSG_ERROR);
    }
    flags&=~EF_SheetLoaded;
    return;
}

bool ExcelApp::__Excel_slot_Excel_load(const QString & excAddr,
                        const QString & templName)
{
    __message("Excel data loading started;  ", scr_INFO);
    QString msg("\t(__Excel_slot_Excel_load):");

    __Excel_slot_Excel_clear();
    __message(msg+"Open Excel application.", scr_INFO);

    AX.mainExcel = new QAxObject("Excel.Application",0);
    if(!AX.mainExcel)
    {
        __message(msg+" Can't open Excel application; ", scr_ERROR);
        return false;
    }
    __connect_toCatch(AX.mainExcel);

    AX.workbooks = AX.mainExcel->querySubObject("Workbooks");
    if(!AX.workbooks)
    {
        __message(msg+"!ERROR Can't load workbooks from Excel; ", scr_ERROR);
        return false;
    }
    __connect_toCatch(AX.workbooks);
    __message(msg+"Open workbook ["+excAddr+"]. ", scr_INFO);
    AX.active_workbook=AX.workbooks->querySubObject("Open (const QString&)", excAddr);
    if(!AX.active_workbook)
    {
        __message(msg+"!ERROR Can't open Excel file ["+excAddr+"]; ", scr_ERROR);
        return false;
    }
    __connect_toCatch(AX.active_workbook);

    AX.worksheets=AX.active_workbook->querySubObject("Worksheets");
    if(!AX.worksheets)
    {
        __message(msg+"!ERROR Can't load worksheets from Excel; ", scr_ERROR);
        return false;
    }
    __connect_toCatch(AX.worksheets);

    __message(msg+"Open template sheet ["+templName+"]; ", scr_INFO);
    AX.template_worksheet = AX.worksheets->querySubObject("Item(const QString&)",templName);
    if(!AX.template_worksheet)
    {
        __message(msg+"!ERROR Can't find template worksheet ["+
                  templName+"] in a file ["+
                  excAddr+"]; ", scr_ERROR);
        return false;
    }
    __connect_toCatch(AX.template_worksheet);

    flags|=EF_ExcelLoaded;
    __message("Excel data loading finished successfully; ", scr_INFO);
    return true;


}


///----------------------LOADING-DATA-FROM-EXCEL------------------------------
bool ExcelApp::__Excel_slot_Data_load()
{
    __message("__Excel_slot_Data_load started; ", scr_INFO);
    QString msg("\t(__Excel_slot_Data_load):");

    if((!(flags|EF_ExcelLoaded))||(!(flags|EF_SheetLoaded)))
    {
        __message(msg+"!ERROR Excel data wasn't load; ", scr_INFO);
        return false;
    }

    __message(msg+"Clear active range variable.", scr_INFO);
    if(AX.active_range)
    {
        delete AX.active_range;
        AX.active_range = nullptr;
    }

    QAxObject * rows = nullptr;
    QAxObject * columns = nullptr;

    bool aok = true;

    for(;;)
    {
        __message(msg+"Getting active range from active worksheet;", scr_INFO);
        AX.active_range =AX.active_worksheet->querySubObject("UsedRange");
        if(!AX.active_range)
        {
            __message(msg+"!ERROR Working range is empty;", scr_ERROR);
            aok=false;
            break;
        }

        __message(msg+"Getting row and column count of the active range", scr_INFO);
        rows = AX.active_range->querySubObject("Rows");
        columns = AX.active_range->querySubObject("Columns");
        if((!rows)||(!columns))
        {
            __message(msg+" Can't calculate a row/column count;", scr_ERROR);
            aok=false;
            break;
        }

        row_count = rows->dynamicCall("Count()").toInt();
        column_count = columns->dynamicCall("Count()").toInt();
        __message(msg+"Active range column count ["+
                  QString::number(column_count)+
                  "] and row count ["+
                  QString::number(row_count)+"];", scr_INFO);

        delete rows; rows=nullptr;
        delete columns; columns=nullptr;

        __message(msg+"Loading data from active range;", scr_INFO);
        QVariant rangeVal = AX.active_range->property("Value");
        if(!rangeVal.isValid())
        {
            __message(msg+"!ERROR Can't get a data batch from Excel file;", scr_ERROR);
            aok=false;
            break;
        }

        QList<QVariant> middleData(std::move(rangeVal.toList()));
        QList<QVariant> headerData(std::move(middleData.at(0).toList()));

        __message(msg+"Analyzing header data from active range;", scr_INFO);
        if(!__Excel_slot_Parse_header(headerData))
        {
            __message(msg+"!ERROR Header data can't be analysed';", scr_ERROR);
            aok=false;
            break;
        }
        __message(msg+"Analyzing main data from active range;", scr_INFO);
        if(!__Excel_slot_Parse_data(middleData))
        {
            __message(msg+"!ERROR Main data can't be analysed';", scr_ERROR);
            aok=false;
            break;
        }
        break;
    }

    if(rows)
    {
        delete rows;
        rows = nullptr;
    }
    if(columns)
    {
        delete columns;
        columns=nullptr;
    }
    if(aok)
    {
        __message("__Excel_slot_Data_load finished successfully.", scr_INFO);
        flags |= EF_DataLoaded;
    }
    else
    {
        __message("__Excel_slot_Data_load finished with isslues.", scr_ERROR);
    }
    return aok;
}

bool ExcelApp::__Excel_slot_Parse_header(const QList<QVariant> & header)
{
    __message("__Excel_slot_Parse_header started;", scr_INFO);
    QString msg("\t\t(__Excel_slot_Parse_header)");

    QRegExp l_test_find("^[Tt]est\\#$");
    QRegExp l_configs_find("^[Cc]onfig$");
    QRegExp l_script_find("^[Ss]cript$");
    QRegExp l_input_find("^[Ii]nput$");
    QRegExp l_min_range("^[Mm]in[lL]im$");
    QRegExp l_max_range("^[Mm]ax[lL]im$");

    __message(msg+"Start to find embedded columns;", scr_INFO);
    for(int i = 0; i < column_count;i++)
    {
        QString info = header.at(i).toString();
        if((columnNumbers[HeadCol_num]<0)&&(l_test_find.indexIn(info)!=-1))
        {
            columnNumbers[HeadCol_num]=i+1;
            __message(msg+"Column with test numbers was found [#"+
                               QString::number(i+1)+"]. ", scr_INFO);
        }
        else if((columnNumbers[HeadCol_cfg]<0)&&(l_configs_find.indexIn(info)!=-1))
        {
            columnNumbers[HeadCol_cfg]=i+1;
            __message(msg+"Column with test configurations was found [#"+
                               QString::number(i+1)+"]. ", scr_INFO);
        }
        else if((columnNumbers[HeadCol_test]<0)&&(l_script_find.indexIn(info)!=-1))
        {
            columnNumbers[HeadCol_test]=i+1;
            __message(msg+"Column with test filenames was found [#"+
                               QString::number(i+1)+"]. ", scr_INFO);
        }
        else if((columnNumbers[HeadCol_var]<0)&&(l_input_find.indexIn(info)!=-1))
        {
            columnNumbers[HeadCol_var]=i+1;
            __message(msg+"Column with test variables was found [#"+
                               QString::number(i+1)+"]. ", scr_INFO);
        }
        else if((columnNumbers[HeadCol_min_range]<0)&&(l_min_range.indexIn(info)!=-1))
        {
            columnNumbers[HeadCol_min_range]=i+1;
            __message(msg+"Column with minimal range was found [#"+
                               QString::number(i+1)+"]. ", scr_INFO);
        }
        else if((columnNumbers[HeadCol_max_range]<0)&&(l_max_range.indexIn(info)!=-1))
        {
            columnNumbers[HeadCol_max_range]=i+1;
            __message(msg+"Column with maximum range was found [#"+
                               QString::number(i+1)+"]. ", scr_INFO);
        }
        else if(columnNumbers[HeadCol_result]<0)
        {
            bool aok = true;
            current_test=info.toInt(&aok)+1;
            if(aok)
            {
                columnNumbers[HeadCol_result]=i+1;
                __message(msg+"Column with last tests was found [#"+
                                   QString::number(i+1)+"]. Test numbers"
                                   "starts with N["+QString::number(
                                       current_test)+"]. ", scr_INFO);
            }
            else
            {
                current_test=1;
            }
        }
        if((columnNumbers[HeadCol_num]>0)&&
                (columnNumbers[HeadCol_cfg]>0)&&
                (columnNumbers[HeadCol_test]>0)&&
                (columnNumbers[HeadCol_var]>0)&&
                (columnNumbers[HeadCol_result]>0)&&
                (columnNumbers[HeadCol_min_range]>0)&&
                (columnNumbers[HeadCol_max_range]>0))
        {
            break;
        }
    }

    __message(msg+"Check previous test results;", scr_INFO);
    if(columnNumbers[HeadCol_result]<0)
    {
        columnNumbers[HeadCol_result]=column_count+1;
        __message(msg+"There is no previous tests, results start to"
                           " write in column N["+
                  QString::number(columnNumbers[HeadCol_result])+
                           "]. ", scr_INFO);
    }

    __message(msg+"Check if all embedded columns was found;", scr_INFO);

    for(int i = 0; i < HeadCol_SIZE;i++)
    {
        if(columnNumbers[i]<0)
        {
             __message(msg+"!ERROR not enought title markers;", scr_ERROR);
             return false;
        }
    }
    __message("__Excel_slot_Parse_header finished successfully;", scr_INFO);
    return true;
}

bool ExcelApp::__Excel_slot_Parse_data(const QList<QVariant> & m_data)
{
    __message("__Excel_slot_Parse_data started; ", scr_INFO);
    QString msg("\t\t(__Excel_slot_Parse_data):");
    _cell_nums.resize(row_count);
    _cell_nums.fill(_int_ERROR);
    for(int i = 1; i < row_count;i++)
    {
        QString n_line(QString::number(i+1));
        __message(msg+" Analyzing line ["+n_line+"] ", scr_INFO);

        QList<QVariant> currentData(std::move(m_data.at(i).toList()));
        int testNum = __Excel_parse_testnum(currentData.at(columnNumbers[HeadCol_num]-1));
        if(testNum>0)
        {
            QString textNum(QString::number(testNum));
            _cell_nums[i]=testNum;
            TESTblock data;
            data.rownumber = i+1;
            data._nts_fileAddr = __Excel_parse_scriptAdr(currentData.at(columnNumbers[HeadCol_test]-1));

            if(data._nts_fileAddr.isEmpty())
            {
                __message(msg+"!ERROR Line ["+n_line+"], test N["+textNum+
                          "] without test name. ", scr_ERROR);
                return false;
            }
            data._hex_fileAddr = __Excel_parse_confAdr(currentData.at(columnNumbers[HeadCol_cfg]-1));

            if(data._hex_fileAddr.isEmpty())
            {
                __message(msg+"!WARNING Line ["+n_line+"], test N["+textNum+
                          "] without config address. ", scr_WARNING);
            }

            data._var_toLoad = __Excel_parse_vars(currentData.at(columnNumbers[HeadCol_var]-1));
            if(data._var_toLoad=="error")
            {
                __message(msg+"!ERROR Line ["+n_line+"], test N["+textNum+
                          "] isslues when parse variables. ", scr_ERROR);
                return false;
            }
            data._min_range=__Excel_parse_range(currentData.at(columnNumbers[HeadCol_min_range]-1));
            if(data._min_range.isNull())
            {
                __message(msg+"!ERROR Line ["+n_line+"], test N["+textNum+
                          "] isslues when parse minimal range. ", scr_ERROR);
                return false;
            }
            data._max_range=__Excel_parse_range(currentData.at(columnNumbers[HeadCol_max_range]-1));
            if(data._max_range.isNull())
            {
                __message(msg+"!ERROR Line ["+n_line+"], test N["+textNum+
                          "] isslues when parse maximal range. ", scr_ERROR);
                return false;
            }
            _test_map.insert(testNum,data);
            __message(msg+"Line ["+n_line +"], test N["+textNum+
                               "] successfully analysed. ", scr_INFO);
        }
        else
        {
            __message("Line ["+n_line+"] have no data. ", scr_INFO);
        }
    }
    _test_iter=_test_map.begin();
    __message("__Excel_slot_Parse_data finished; ", scr_INFO);
    return true;
}

int ExcelApp::__Excel_parse_testnum(const QVariant & testnum) const
{
    bool aok = true;
    QString dataCell = testnum.toString();
    int testNum = dataCell.toInt(&aok);
    if(aok&&(testNum>0))
    {
        return testNum;
    }
    return -1;
}

QString ExcelApp::__Excel_parse_scriptAdr(const QVariant & addr)
{
    QString msg("\t\t\t(__Excel_parse_scriptAdr):");
    QRegExp find_var("^\\$\\w+$");
    QString address = addr.toString();

    if(address.isEmpty())
    {
        __message(msg+"!ERROR Line without test name!", scr_ERROR);

    }
    else if(find_var.indexIn(address)!=-1)
    {
        __message(msg+"Line is variable ["+address+"] to read.", scr_INFO);
    }
    else
    {
        address = _script_dir+"/"+address+".nts";
        if(__Excel_check_FileAddr(address))
        {
            __message(msg+"Line file ["+address+"] is exist. ",
                               scr_INFO);
        }
        else
        {
            __message("!WARNING Line file ["+address+"] not exist, "
                               "or can't be reached. ", scr_WARNING);
        }
    }
    return address;
}

QString ExcelApp::__Excel_parse_confAdr(const QVariant & addr)
{
    QString msg("\t\t\t(__Excel_parse_scriptAdr):");
    QString address = addr.toString();
    if(address.isEmpty()||(address=="-"))
    {
        __message(msg+"!WARNING Line without config name!", scr_WARNING);
        address.clear();
        return address;
    }
    else
    {
        address = _conf_dir+"/"+address+".hex";
        if(__Excel_check_FileAddr(address))
        {
            __message(msg+"Line file ["+address+"] is exist. ",
                               scr_INFO);
        }
        else
        {
            __message(msg+"!WARNING Line file ["+address+"] not exist, "
                               "or can't be reached. ", scr_WARNING);
        }
    }
    return address;
}

QString ExcelApp::__Excel_parse_vars(const QVariant & vars)
{
    QString msg("\t\t\t(__Excel_parse_vars):");

    QString variables = vars.toString();
    if(variables.isEmpty()||(variables=="-"))
    {
        __message(msg+"Line have no variables; ", scr_INFO);
        return variables;
    }
    QRegExp find_str("\"(.+)\"");
    QRegExp find_var("^\\$(w+)$");
    QRegExp find_bool("([Tt]rue|[Ff]alse|TRUE|FALSE)");
    QStringList list = variables.split(',',QString::SkipEmptyParts);
    __message(msg+"Start to parse line ["+variables+"]; ", scr_INFO);
    variables.clear();
    for(int i = 0; i<list.size();i++)
    {
        if(!variables.isEmpty())
        {
            variables+=',';
        }
        QString cur_var =  list.at(i);
        __message("\t"+msg+"Current element is ["+cur_var+"]; ", scr_INFO);
        if(find_bool.indexIn(cur_var)!=-1)
        {
            __message(msg+" Line variable ["+cur_var+
                      "] interpritated as a boolean; ", scr_INFO);
            variables+="[b]("+find_var.cap(1).toLower()+")";
        }
        else if(find_var.indexIn(cur_var)!=-1)
        {
            __message(msg+" Line variable ["+cur_var+
                      "] interpritated as script variable; ", scr_INFO);
            variables+="[v]("+find_var.cap(1)+")";
        }
        else if(find_str.indexIn(cur_var)!=-1)
        {
            __message(msg+" Line variable ["+cur_var+
                      "] interpritated as string; ", scr_INFO);
            variables+="[s]("+find_str.cap(1)+")";
        }
        else
        {
            QRegExp find_int("(?:(?:\\-?\\d+)|(?:0[xX][0-9A-Fa-f]+))$");
            QRegExp find_float("(\\-?\\d+\\.\\d+(?:[eE]\\-?\\d+)?)$");
            bool aok = true;
            QString type;
            if(find_int.indexIn(cur_var)!=-1)
            {
                int var = cur_var.toInt(&aok,0);
                type = 'i';
            }
            else if(find_float.indexIn(cur_var)!=-1)
            {
                double var = cur_var.toDouble(&aok);
                type = 'd';
            }
            if(!aok)
            {
                __message(msg+"!ERROR Line variable ["+cur_var+
                          "] can't be interpritate. ", scr_ERROR);
                return "error";
            }
            __message(msg+" Line variable ["+cur_var+
                      "] interpritated as double; ", scr_INFO);
            variables+="["+type+"]("+cur_var+")";
        }
    }
    return variables;
}

QVariant ExcelApp::__Excel_parse_range(const QVariant & value)
{
    QString msg("\t\t\t(__Excel_parse_range):");
    __message(msg+"is starting; ", scr_INFO);
    QRegExp find_int("^(?:(?:\\-?\\d+)|(?:0[xX][0-9A-Fa-f]+))$");
    QRegExp find_double("^(\\-?\\d+\\.\\d+(?:[eE][\\-\\+]?\\d+)?)$");
    QRegExp find_bool("([Tt]rue|[Ff]alse|TRUE|FALSE)");
    QRegExp find_str("\"(.+)\"");
    QString line(value.toString());
    __message(msg+"Excel core is trying to describe a value ["+line+"]; ", scr_INFO);

    QVariant answer;
    if(find_int.indexIn(line)>=0)
    {
        bool aok = true;
        int temp = value.toInt(&aok);
        if(aok)
        {
            __message(msg+"Excel core is describing the value as integer; ", scr_INFO);
            answer=temp;
        }
    }
    else if(find_double.indexIn(line)>=0)
    {
        bool aok = true;
        double temp = value.toDouble(&aok);
        if(aok)
        {
            __message(msg+"Excel core is describing the value as double; ", scr_INFO);
            answer=temp;
        }
    }
    else if(find_bool.indexIn(line)>=0)
    {
        __message(msg+"Excel core is describing the value as boolean; ", scr_INFO);
        answer = value.toBool();
    }
    else if(find_str.indexIn(line)>=0)
    {
        __message(msg+"Excel core is describing the value as string; ", scr_INFO);
        answer = line.remove("\"");
    }
    else
    {
        __message(msg+"Excel core can't discribe the value; ", scr_WARNING);
    }
    __message(msg+"is finishing; ", scr_INFO);
    return answer;
}

bool ExcelApp::__Excel_check_FileAddr(const QString & fileaddr) const
{
     if(fileaddr.isEmpty())
     {
         return false;
     }
     QFileInfo fileinfo(fileaddr);
     return (fileinfo.isFile()&&fileinfo.isReadable());
}


///---------------------------SEQUANCES MANIPULATIONS-------------------------
void ExcelApp::Excel_slot_append_test_seq_range(int start_num , int finish_num)
{
    __message("Excel_slot_append_test_seq_range started;",scr_INFO);
    QString msg("\t(Excel_slot_append_test_seq_range):");

    if(flags&EF_TestActive)
    {
        __message(msg+"!ERROR You can't append tests when test is running';",scr_ERROR);
        return;
    }

    if(!(flags&(EF_ExcelLoaded|EF_SheetLoaded|EF_DataLoaded)))
    {
        __message(msg+"!ERROR Excel data wasn't load;",scr_ERROR);
        __Excel_slot_Seq_clear();
        flags&=~EF_SeqLoaded;
        return;
    }

    if(userflags&ExApp_AutoClearSeq)
    {
        __message(msg+"Previous sequance is cleared;",scr_INFO);
        __Excel_slot_Seq_clear();
    }

    if(start_num<0)
    {
        __message(msg+"All test will be load;",scr_INFO);
        _test_iter = _test_map.begin();
        while(_test_iter!=_test_map.end())
        {
            _sequance._tests.append(_test_iter.key());
            _test_iter++;
        }
    }
    else
    {

        _test_iter = _test_map.find(start_num);
        if(_test_iter==_test_map.end())
        {
            __message(msg+"!ERROR there is no test N["+
                      QString::number(start_num)+"];",scr_ERROR);
            return;
        }



        if(start_num==finish_num)
        {
            __message(msg+"One test N["+QString::number(start_num)+"] load;",scr_INFO);
            _sequance._tests.append(start_num);

        }
        else if(start_num<finish_num)
        {
            __message(msg+"Upward test range N["+QString::number(start_num)+
                      "] - ["+QString::number(finish_num)+"] load;",scr_INFO);
            while(_test_iter!=_test_map.end())
            {
                if(_test_iter.key()>finish_num)
                {
                    break;
                }
                _sequance._tests.append(_test_iter.key());
                _test_iter++;
            }
        }
        else
        {
            __message(msg+"Downward test range N["+QString::number(start_num)+
                      "] - ["+QString::number(finish_num)+"] load;",scr_INFO);
            do
            {
                if(_test_iter.key()<finish_num)
                {
                    break;
                }
                _sequance._tests.append(_test_iter.key());
                _test_iter--;
            }
            while(_test_iter!=_test_map.begin());

        }
    }
    __message(msg+"Sequance iterator set to start position;",scr_INFO);
    _sequance._cur_test = _sequance._tests.begin();
    _sequance._repeats = 1;
    flags|=EF_SeqLoaded;
     __message("Excel_slot_append_test_seq_range finished;",scr_INFO);
    return;
}

void ExcelApp::Excel_slot_remove_test_seq_range(int start_num, int finish_num)
{
    __message("Excel_slot_remove_test_seq_range started;",scr_INFO);
    QString msg("\t(Excel_slot_remove_test_seq_range):");

    if(flags&EF_TestActive)
    {
        __message(msg+"!ERROR You can't remove tests when test is running';",scr_ERROR);
        return;
    }

    if(!(flags&(EF_ExcelLoaded|EF_SheetLoaded|EF_DataLoaded|EF_SeqLoaded)))
    {
        __message(msg+"!ERROR Excel data wasn't load;",scr_ERROR);
        __Excel_slot_Seq_clear();
        flags&=~EF_SeqLoaded;
        return;
    }
    __message(msg+"Remove test range N["+QString::number(start_num)+
              "] - ["+QString::number(finish_num)+"] from sequance;",scr_INFO);
    if(start_num<=finish_num)
    {
        do
        {
            __message(msg+"Removed test N ["+QString::number(start_num)+
                      "] have ["+_sequance._tests.removeAll(start_num)+
                      "] incomings in sequance;",scr_INFO);
            start_num++;
        }
        while(start_num<=finish_num);
    }
    else
    {
        do
        {
            __message(msg+"Removed test N ["+QString::number(start_num)+
                      "] have ["+_sequance._tests.removeAll(start_num)+
                      "] incomings in sequance;",scr_INFO);
            _sequance._tests.removeAll(start_num);
            start_num--;
        }
        while(start_num>=finish_num);
    }
    if(_sequance._tests.isEmpty())
    {
        __message(msg+"Sequance now is empty;",scr_INFO);
        flags&=~EF_SeqLoaded;
    }
    __message(msg+"Sequance iterator set to start position;",scr_INFO);
    _sequance._cur_test = _sequance._tests.begin();
    _sequance._repeats = 1;
    __message("Excel_slot_remove_test_seq_range finished;",scr_INFO);
    return;
}

void ExcelApp::Excel_slot_append_debug_pause(int test_num)
{
    __message("Excel_slot_append_debug_pause started;",scr_INFO);
    QString msg("\t(Excel_slot_append_debug_pause):");

    if(!(flags&(EF_ExcelLoaded|EF_SheetLoaded|EF_DataLoaded)))
    {
        __message(msg+"!ERROR Excel data wasn't load;",scr_ERROR);
        return;
    }

    if(flags&EF_TestActive)
    {
        __message(msg+"!ERROR You can't append pauses when test is running';",scr_ERROR);
        return;
    }

    if(!_test_map.contains(test_num))
    {
        __message(msg+"!ERROR test list don't have test N["+QString::number(test_num)+"];",scr_ERROR);
        return;
    }
    if(_sequance._paused.contains(test_num))
    {
        __message(msg+"WARNING! pause on test N["+QString::number(test_num)+"] already set;",scr_WARNING);
    }
    else
    {
        __message(msg+"pause on test N["+QString::number(test_num)+"] is set;",scr_INFO);
        _sequance._paused.insert(test_num);
    }
    __message("Excel_slot_append_debug_pause finished;",scr_INFO);
}

void ExcelApp::Excel_slot_set_repeat_number(int repeat)
{
    __message("Excel_slot_set_repeat_number started;",scr_INFO);
    QString msg("\t(Excel_slot_set_repeat_number):");

    if(!(flags&(EF_ExcelLoaded|EF_SheetLoaded|EF_DataLoaded|EF_SeqLoaded)))
    {
        __message(msg+"!ERROR Excel data wasn't load;",scr_ERROR);
        return;
    }

    if(flags&EF_TestActive)
    {
        __message(msg+"!ERROR You can't remove pauses when test is running';",scr_ERROR);
        return;
    }

    if(flags&EF_TestActive)
    {
        __message(msg+"!ERROR You can't change repeats when test is running';",scr_ERROR);
        return;
    }

    if(repeat<1)
    {
        __message(msg+"!ERROR repeat value must be greater that 0 ;",scr_ERROR);
        return;
    }

     __message(msg+"Current test sequance would be repeated ["+
               QString::number(repeat)+"] times;",scr_INFO);
    _sequance._repeats=repeat;
    return;
    __message("Excel_slot_set_repeat_number finished;",scr_INFO);
}

void ExcelApp::Excel_slot_delete_debug_pause(int test_num)
{
    __message("Excel_slot_append_debug_pause started;",scr_INFO);
    QString msg("\t(Excel_slot_append_debug_pause):");

    if(!(flags&(EF_ExcelLoaded|EF_SheetLoaded|EF_DataLoaded)))
    {
        __message(msg+"!ERROR Excel data wasn't load;",scr_ERROR);
        return;
    }

    if(test_num==-1)
    {
        __message(msg+"All test pauses are removed;",scr_INFO);
        _sequance._paused.clear();
    }
    else
    {
        if(!_test_map.contains(test_num))
        {
            __message(msg+"!ERROR test list don't have test N["+
                      QString::number(test_num)+"];",scr_ERROR);
            return;
        }
        else
        {
            __message(msg+"Test pause N["+QString::number(test_num)+
                      "] are removed;",scr_INFO);
            _sequance._paused.remove(test_num);
        }

    }
    __message("Excel_slot_append_debug_pause finished;",scr_INFO);
}


///---------------------CLEANING-FUNCTIONS------------------------
void ExcelApp::Excel_slot_clear()
{
    __Excel_slot_Excel_clear();
}

void ExcelApp::Excel_slot_clear_test_seq()
{
    __message("Excel_slot_clear_test_seq started;",scr_INFO);
    if(flags&EF_TestActive)
    {
        __message("\t(Excel_slot_clear_test_seq)"
                  "!ERROR You can't append tests when "
                  "test is running';",scr_ERROR);
        return;
    }
    __Excel_slot_Seq_clear();
    __message("Excel_slot_clear_test_seq finished;",scr_INFO);
}

void ExcelApp::__Excel_slot_Excel_clear()
{

    __message("Closing Excel application started:  ", scr_INFO);
    __Excel_slot_Data_clear();
    if(AX.active_range)
    {
        __disconnect_fromCatch(AX.active_range);
        delete AX.active_range;
        AX.active_range = nullptr;
        __message("Active range closed.  ", scr_INFO);
    }
    if(AX.active_worksheet)
    {
        __disconnect_fromCatch(AX.active_worksheet);
        delete AX.active_worksheet;
        AX.active_worksheet = nullptr;
        __message("Active worksheet closed.  ", scr_INFO);
    }
    if(AX.template_worksheet)
    {
        __disconnect_fromCatch(AX.template_worksheet);
        delete AX.template_worksheet;
        AX.template_worksheet=nullptr;
        __message("Template worksheet closed.  ", scr_INFO);
    }
    if(AX.worksheets)
    {
        __disconnect_fromCatch(AX.worksheets);
        delete AX.worksheets;
        AX.worksheets=nullptr;
        __message("Worksheets closed.  ", scr_INFO);
    }
    if(AX.active_workbook)
    {
        AX.active_workbook->dynamicCall("Save()");
        __disconnect_fromCatch(AX.active_workbook);
        delete AX.active_workbook;
        AX.active_workbook = nullptr;
        __message("Active workbook closed.  ", scr_INFO);
    }
    if(AX.workbooks)
    {
        AX.workbooks->dynamicCall("Close()");
        __disconnect_fromCatch(AX.workbooks);
        delete AX.workbooks;
        AX.workbooks = nullptr;
        __message("Workbooks closed.  ", scr_INFO);
    }
    if(AX.mainExcel)
    {
        AX.mainExcel->dynamicCall("Quit()");
        __disconnect_fromCatch(AX.mainExcel);
        delete AX.mainExcel;
        AX.mainExcel = nullptr;
        __message("Application closed.  ", scr_INFO);
    }
    flags&=~(EF_ExcelLoaded|EF_SheetLoaded);
    //userflags=ExApp_Default;
    return;

}

void ExcelApp::__Excel_slot_Data_clear()
{
    __message("Excel data reset started:  ", scr_INFO);
    QString msg("\t(Excel_slot_Data_clear):");
    _test_map.clear();
    _test_iter = _test_map.begin();

    __message(msg+"Test data cleaned.  ", scr_INFO);

    _debug_list.clear();
    row_count = 0;
    column_count = 0;
    current_test = 0;
    __message(msg+"Temporary data reset.  ", scr_INFO);

    for(int step = 0; step<HeadCol_SIZE;step++)
    {
        columnNumbers[step] = -1;
    }
    __message(msg+"Numbers of working columns reset.  ", scr_INFO);

    flags&=~(EF_DataLoaded);
    __Excel_slot_Seq_clear();
    __message(msg+"User sequences removed. ", scr_INFO);
    __message("Excel data successfully reset:  ", scr_INFO);
    return;
}

void ExcelApp::__Excel_slot_Seq_clear()
{
    _sequance._tests.clear();
    _sequance._cur_test=_sequance._tests.begin();
    _sequance._paused.clear();
    _sequance._repeats = 0;
    flags&=~EF_SeqLoaded;
}

void ExcelApp::__Excel_slot_result_clear()
{
    __message("__Excel_slot_result_clear started;", scr_INFO);
    _test_iter = _test_map.begin();
    while(_test_iter!=_test_map.end())
    {
        _test_iter.value()._result.clear();
        _test_iter++;
    }
    __message("__Excel_slot_result_clear finished;", scr_INFO);
}

void ExcelApp::__Excel_slot_start_clear()
{
    flags = 0;
    userflags=ExApp_Default;
    AX.active_range=nullptr;
    AX.active_workbook=nullptr;
    AX.active_worksheet=nullptr;
    AX.mainExcel=nullptr;
    AX.template_worksheet=nullptr;
    AX.workbooks=nullptr;
    AX.worksheets=nullptr;
}

///---------------------EXCTERNAL COMMUNICATIONS-------------------
void ExcelApp::Excel_slot_start_tests()
{
    __message("Excel_slot_start_tests started;",scr_INFO);
    QString msg("\t(Excel_slot_start_tests):");
    if(!(flags&(EF_ExcelLoaded|EF_SheetLoaded|EF_DataLoaded|EF_SeqLoaded)))
    {
        __message(msg+"!ERROR One ore more elements wasn't load: "
                      "/Excel/worksheet/data/sequance;",scr_ERROR);
        return;
    }

    if(flags&EF_TestActive)
    {
        __message(msg+"!ERROR Tests already running;",scr_ERROR);
        return;
    }

    if(_sequance._tests.isEmpty())
    {
        __message(msg+"!ERROR the sequance of tests is not set;",scr_ERROR);
        return;
    }
    if(userflags&ExApp_QuickResult)
    {
        __Excel_write_quickHeader();
    }
    flags|=EF_TestActive;
    __message("STARTING TEST SEQUANCE:\n\tTest count:"+
              QString::number(_sequance._tests.size())+
                "\n\tRepeat:"+QString::number(
                  _sequance._repeats),scr_WARNING);
    _sequance._cur_test=_sequance._tests.begin();
    if(_sequance._paused.contains(_sequance._cur_test.operator *()))
    {
        __message(msg+"The first test"
                      " required for pause;",scr_INFO);
        emit Excel_signal_setPause(true);
    }
    if(!__Excel_active_contunue())
    {
        __Excel_active_finish();
    }

}

bool ExcelApp::__Excel_active_contunue()
{
    __message("__Excel_active_contunue started;",scr_INFO);
    QString msg("\t\t(__Excel_active_contunue):");

    if(!(flags&EF_TestActive))
    {
        __message(msg+"!ERROR Unexpected condition - test isn't active;",scr_ERROR);
        return false;
    }

    if(flags&EF_BreakActive)
    {
        __message(msg+"Break State is active;",scr_INFO);
        return false;
    }

    int cur_test = _sequance._cur_test.operator *();
    __message(msg+"Current test number ["+
              QString::number(cur_test)+"];",scr_INFO);
    _test_iter = _test_map.find(cur_test);
    if(_test_iter==_test_map.end())
    {
        __message(msg+"!ERROR test N["+QString::number(_test_iter.key())+
                  "] can't be find in loaded tests;",scr_ERROR);
        return false;
    }
    TESTblock & cur_block= _test_iter.value();
    __message(msg+"Current test data: \n"
                  "\t\t\tTest address: "+cur_block._nts_fileAddr+";\n"+
                  "\t\t\tConfig address: "+cur_block._hex_fileAddr+";\n"+
                  "\t\t\tVariables: "+cur_block._var_toLoad+";",scr_INFO);

    emit Excel_signal_next(cur_block._nts_fileAddr,
                           cur_block._hex_fileAddr,
                           cur_block._var_toLoad,
                           _test_iter.key());
    __message(msg+"Package was send to autotest;",scr_INFO);
    __message("__Excel_active_contunue finished;",scr_INFO);
    return true;
}

void ExcelApp::Excel_slot_test_complete(QVariant answer)
{
    __message("Excel_slot_test_complete invoked;",scr_INFO);
    QString msg("\t\t(Excel_slot_test_complete):");

    if(flags&EF_BreakActive)
    {
        __message(msg+"Break State is active;",scr_INFO);
        __Excel_active_finish();
        return;
    }

    if(!(flags&EF_TestActive))
    {
        __message(msg+"!WARNING - result ["+answer.toString()+"] came when excel isn't active;",scr_WARNING);
        __Excel_active_finish();
        return;
    }
    if((!answer.isValid())||(answer.isNull()))
    {
        __message(msg+"!WARNING result of test N["+
                  QString::number(_test_iter.key())+
                  "] is not valid;",scr_WARNING);
    }
    else
    {
        __message(msg+"Excel Core is checking result's range;",scr_INFO);
        char norangemiss = __Excel_check_result(_test_iter.value()._min_range,
                                                _test_iter.value()._max_range,
                                                answer);
        _test_iter.value()._result = answer;
        _test_iter.value()._range_ok = norangemiss;
        __message(msg+"Script result set as ["+answer.toString()+"];",scr_INFO);
        if(userflags&ExApp_QuickResult)
        {
            __Excel_write_quickResult(_test_iter.value());
        }

        if(userflags&ExApp_BreakAfterMiss)
        {
            if(norangemiss)
            {
                 __message(msg+"ERROR! Script's answer is out of range;",scr_ERROR);
                __Excel_active_finish();
                return;
            }
        }
    }
    __message(msg+"Prepare to a next iteration; ",scr_INFO);
    _sequance._cur_test++;
    if(_sequance._cur_test==_sequance._tests.end())
    {
        __message(msg+"Sequance is finished;",scr_INFO);
        __Excel_active_finish();
    }
    else
    {
        int cur_test = _sequance._cur_test.operator *();
        if(_sequance._paused.contains(cur_test))
        {
            __message(msg+"Test N["+QString::number(cur_test)+"]"
                          " required for pause;",scr_INFO);
            emit Excel_signal_setPause(true);
        }
        if(!__Excel_active_contunue())
        {
            __Excel_active_finish();
        }
    }
    __message("Excel_slot_test_complete finished;",scr_INFO);
}

char ExcelApp::__Excel_check_result(const QVariant & min,
                                    const QVariant & max,
                                    const QVariant & result)
{
    const char lesser=-1;
    const char greater=1;
    const char nonequial=2;
    const char fine = 0;
    QString msg("(__Excel_check_result):");
    __message(msg+"is starting;",scr_INFO);
    QString v_type(min.typeName());
    if(v_type=="int")
    {
        __message(msg+"variable type is declared as integer ;",scr_INFO);
        int imin = min.toInt();
        int imax = max.toInt();
        int control = result.toInt();
        if(imin>control)
        {
            return lesser;
        }
        else if(imax<control)
        {
            return greater;
        }
        else
        {
            return fine;
        }
        //return ((imin<=control)&&(imax>=control));
    }
    else if(v_type=="double")
    {
        __message(msg+"variable type is declared as double ;",scr_INFO);
        double imin = min.toDouble();
        double imax = max.toDouble();
        double control = result.toDouble();
        if(imin>control)
        {
            return lesser;
        }
        else if(imax<control)
        {
            return greater;
        }
        else
        {
            return fine;
        }
        //return ((imin<=control)&&(imax>=control));
    }
    else if(v_type=="bool")
    {
        __message(msg+"variable type is declared as bool ;",scr_INFO);
        bool logmin = min.toBool();
        bool logmax = max.toBool();
        bool control = result.toBool();
        if((logmin==control)&&(logmax==control))
        {
            return fine;
        }
        else
        {
            return nonequial;
        }
        //return logmin&&logmax&&control;
    }
    else if(v_type=="QString")
    {
        __message(msg+"variable type is declared as string ;",scr_INFO);
        QString strmin(min.toString());
        QString strmax(max.toString());
        QString control(result.toString());
        if((control==strmax)&&(strmin==control))
        {
            return fine;
        }
        else
        {
            return nonequial;
        }
        //return (strmin==strmax)&&(strmin==control);
    }
    __message(msg+"WARNING! Variable type doesn't found;",scr_WARNING);
    return nonequial;
}

void ExcelApp::__Excel_active_finish()
{
    if(!(flags&EF_TestActive))
    {
        __message("!WARNING__Excel_active_finish start;",scr_WARNING);
        return;
    }
    __message("__Excel_active_finish start;",scr_INFO);
    QString msg("\t\t(__Excel_active_finish):");
    __message(msg+"Sequence iteration complete;",scr_INFO);
    _sequance._repeats--;
    if((_sequance._repeats<1)||(flags&EF_BreakActive))
    {
        if(!(userflags&ExApp_QuickResult))
        {
            if(flags&EF_BreakActive)
            {
                __message(msg+"Tests sequence was break by user;",scr_WARNING);
                if(userflags&ExApp_WriteResultWhenBreak)
                {
                    __message(msg+"User chose to write result after break;",scr_INFO);
                    __Excel_write_Results();
                }
                //flags&=~EF_BreakActive;
            }
            else
            {
                __message(msg+"Tests sequence was finished;",scr_INFO);
                __Excel_write_Results();
            }
        }

        if(userflags&ExApp_AutoClearSeq)
        {
            __message(msg+"User chose to clear sequance list automaticly. List is clear;",scr_INFO);
            __Excel_slot_Seq_clear();
        }
        else
        {
            __message(msg+"Sequance iterator back to start position;",scr_INFO);
            _sequance._cur_test = _sequance._tests.begin();
            _sequance._repeats=1;
        }
        __message(msg+"Clearing results of the previous testing;",scr_INFO);
        __Excel_slot_result_clear();
        __message(msg+"Testing is over;",scr_INFO);
        flags&=~EF_TestActive;
        //current_test++;
    }
    else
    {
        flags&=~EF_TestActive;
        if(!(userflags&ExApp_QuickResult))
        {
            __Excel_write_Results();
        }
        __message(msg+"Next iteration starts (["+QString::number(_sequance._repeats)+"] remain);",scr_INFO);
        //current_test++;
        Excel_slot_start_tests();
    }
    flags&=~EF_BreakActive;
    return;

}

bool ExcelApp::__Excel_write_quickHeader()
{
     __message("__Excel_write_quickHeader started;",scr_INFO);
     QString msg("\t\t(__Excel_write_quickHeader):");


    current_test = __Excel_slot_Actual_test_number();
    if(current_test==_int_ERROR)
    {
        current_test=0;
        __message(msg+"!ERROR when getting actual test number; ", scr_ERROR);
        return false;
    }
    current_test++;
    QAxObject * column_new = AX.active_worksheet->querySubObject(
                "Columns(int)",columnNumbers[HeadCol_result]);
    if(!column_new)
    {
        __message(msg+ "!ERROR Can't record result cause can't insert new column; ",
                           scr_ERROR);
        return false;
    }
    column_new->dynamicCall("Select");
    column_new->dynamicCall("Insert");
    delete column_new;
    column_new = nullptr;

    column_new = AX.active_worksheet->querySubObject(
                "Cells(QVariant&, QVariant&)",1,columnNumbers[HeadCol_result]);
    if(!column_new)
    {
        current_test=0;
        __message(msg+ "!ERROR Can't record result cause can't insert new column; ",
                           scr_ERROR);
        return false;
    }

    if(column_new->setProperty("Value",current_test))
    {
        __message(msg+"New column #["+QString::number(current_test)+"] inserted; ",
                  scr_INFO);
    }
    else
    {
        __message(msg+"!WARNING New column #["+QString::number(current_test)+"]can't"
                  " be inserted;", scr_WARNING);
    }
    delete column_new;
    column_new = nullptr;

    __message("__Excel_write_quickHeader finished;",scr_INFO);
    return true;

}

bool ExcelApp::__Excel_write_quickResult(const TESTblock & data)
{
    __message("__Excel_write_quickResult started;",scr_INFO);
    QString msg("\t\t(__Excel_write_quickResult):");

    QAxObject * current_cell = AX.active_worksheet->querySubObject(
                "Cells(QVariant,QVariant)",data.rownumber,
                columnNumbers[HeadCol_result]);
    if(!current_cell)
    {
        __message(msg+"!ERROR Can't record result cause can't select cell row["+
                  QString::number(data.rownumber)+"]column["+
                  QString::number(columnNumbers[HeadCol_result])+"]; ",
                  scr_ERROR);
        return false;
    }

    if(current_cell->setProperty("Value",data._result))
    {
        __message(msg+"Data ["+data._result.toString()+"] inserted in cell (row["+
                  QString::number(data.rownumber)+"]column["+
                  QString::number(columnNumbers[HeadCol_result])+"]); ",
                  scr_INFO);
        QAxObject *inheirs=nullptr;
        inheirs = current_cell->querySubObject("Interior");
        if(inheirs)
        {
            QColor color;
            if(!(data._range_ok))
            {
                color = QColor(Qt::white);
            }
            else
            {
                if(data._range_ok==-1)
                {
                    color = QColor("red");
                }
                else if(data._range_ok==1)
                {
                    color = QColor("blue");
                }
                else if(data._range_ok==2)
                {
                    color = QColor(Qt::darkYellow);
                }
            }
            inheirs->setProperty("Color",color);
            delete inheirs;
            inheirs = nullptr;
        }
        else
        {
            __message(msg+"WARNING! Can't color the cell;",scr_WARNING);
        }
        AX.active_workbook->dynamicCall("Save()");
    }
    else
    {
        __message(msg+"!WARNING Data ["+data._result.toString()+"] can't be inserted (row["+
                  QString::number(data.rownumber)+"]column["+
                  QString::number(columnNumbers[HeadCol_result])+"]); ",
                  scr_WARNING);
    }
    delete current_cell;
    current_cell=nullptr;
    __message("__Excel_write_quickResult finished;",scr_INFO);
    return true;
}

bool ExcelApp::__Excel_write_Results()
{
    __message("__Excel_write_Results started;",scr_INFO);
    QString msg("\t\t(__Excel_write_Results):");

    int cur_number = __Excel_slot_Actual_test_number();
    if(cur_number==_int_ERROR)
    {
        __message(msg+"!ERROR when getting actual test number; ", scr_ERROR);
        return false;
    }
    cur_number++;
    QList<QVariant> table;
    QList<QVariant> row;
    row.append(cur_number);
    table.append(QVariant(row));
    row.clear();
    bool aok = true;
     __message(msg+"Start to filling list of variables to insert into Excel table; ", scr_INFO);
    for (int i=1; i<row_count; i++)
    {
        int real_num = _cell_nums.at(i);

        if(real_num!=_int_ERROR)
        {
            _test_iter = _test_map.find(real_num);
            if(_test_iter!=_test_map.end())
            {
                row.append(_test_iter.value()._result);
                aok=aok&&(!_test_iter.value()._range_ok);
                __message("\t"+msg+" in row N["+
                          QString::number(i)+
                          "] - inserting value ["+
                          _test_iter.value()._result.toString()+"]; ", scr_INFO);
            }
            else
            {
                 __message("\t"+msg+"!WARNING in row ["+
                           QString::number(i)+
                           "] can't find test N["+
                           QString::number(real_num)+
                           "] to insert result value; ", scr_WARNING);
                QVariant empty;
                row.append(empty);
            }
        }
        else
        {
            QVariant empty;
            row.append(empty);
        }
        table.append(QVariant(row));
        row.clear();
    }
    row.append(QVariant(aok));
    table.append(QVariant(row));
    /*
     * We can add a colorisation of cells here, but it ruins
     * the idea of the quick write mechanism.
    */
    QAxObject * unified_col =AX.active_worksheet->querySubObject(
                "Columns(int)",columnNumbers[HeadCol_result]);
    if(!unified_col)
    {
        __message(msg+"!ERROR Can't record test results."
                           " Error when working column selected. ",
                           scr_ERROR);
        return false;
    }
    unified_col->dynamicCall("Select");
    unified_col->dynamicCall("Insert");
    delete unified_col;
    unified_col = nullptr;
    __message(msg+"New column was inserted into Excel active sheet; ", scr_INFO);

    unified_col = __Excel_slot_Get_range(1,columnNumbers[HeadCol_result],
                                         row_count+1,columnNumbers[HeadCol_result]);
    if(!unified_col)
    {
        __message(msg+"!ERROR column to record can't be reached; ", scr_ERROR);
        return false;
    }
    __message(msg+"Inserted column was selected; ", scr_INFO);
    unified_col->setProperty("Value",QVariant(table));
    __message(msg+"Data was inserted into column; ", scr_INFO);
    delete unified_col;
    unified_col = nullptr;

    AX.active_workbook->dynamicCall("Save()");
    __message(msg+"Active sheet save changes; ", scr_INFO);
    __message("__Excel_write_Results finished;",scr_INFO);
    return true;
}

int ExcelApp::__Excel_slot_Actual_test_number()
{
    __message("__Excel_slot_Actual_test_number started", scr_INFO);
    QString msg("\t\t\t(__Excel_slot_Actual_test_number):");
   if(!(flags&EF_SheetLoaded))
   {
       __message(msg+"!ERROR Active worksheet not selected; ", scr_ERROR);
       return _int_ERROR;
   }

   QAxObject * cell_with_number = AX.active_worksheet->querySubObject(
               "Cells(QVariant&, QVariant&)",1,columnNumbers[HeadCol_result]);

   if(!cell_with_number)
   {
       __message(msg+"!ERROR Can't select active test number; ", scr_ERROR);
       return _int_ERROR;
   }

   QVariant test_Number( std::move(cell_with_number->property("Value")));
   delete cell_with_number;
   cell_with_number = nullptr;
   bool aok = false;
   int answer = 0;
   if(test_Number.isValid())
   {
       answer = test_Number.toInt(&aok);
       __message(msg+ " Actual test number ["+
                 QString::number(answer)+"]; ", scr_INFO);
   }
   if(!aok)
   {
       answer = 0;
       __message(msg+" No actual test number; ", scr_INFO);
   }
   __message("__Excel_slot_Actual_test_number finished;", scr_INFO);
   return answer;
}

QAxObject * ExcelApp::__Excel_slot_Get_range(  int leftTopRow,
                                int leftTopColumn,
                                int rightBottomRow,
                                int rightBottomColumn)
{
    __message("__Excel_slot_Get_range started;",scr_INFO);
    QString msg("\t\t(__Excel_slot_Get_range):");

    __message(msg+ "Selecting diapasone: r["+
                       QString::number(leftTopRow)+"]c["+
                       QString::number(leftTopColumn)+"]-r["+
                       QString::number(rightBottomRow)+"]c["+
                       QString::number(rightBottomColumn)+"];",scr_INFO);

     if(!(flags&EF_SheetLoaded))
     {
         __message(msg+"!ERROR Active worksheet isn't selected; ",scr_ERROR);
         return nullptr;
     }

     QAxObject * LTcell = nullptr;
     QAxObject * RBcell = nullptr;
     QAxObject * range = nullptr;
     while(true)
     {
        LTcell = AX.active_worksheet->querySubObject("Cells(QVariant&, QVariant&)",
                                               leftTopRow,leftTopColumn);
        RBcell = AX.active_worksheet->querySubObject("Cells(QVariant&, QVariant&)",
                                               rightBottomRow,rightBottomColumn);
        if((!LTcell)||(!RBcell))
        {
            __message(msg+ "!ERROR when program trying to select diapasone: "
                               "r["+QString::number(leftTopRow)+"]c["+
                               QString::number(leftTopColumn)+"]-r["+
                               QString::number(rightBottomRow)+"]c["+
                               QString::number(rightBottomColumn)+"];", scr_ERROR);
            break;
        }
        range = AX.active_worksheet->querySubObject("Range(const QVariant&,const QVariant&",
                                              LTcell->asVariant(),RBcell->asVariant());
        if(!range)
        {
            __message(msg+"!ERROR when program trying to select range: "
                               "r["+QString::number(leftTopRow)+"]c["+
                               QString::number(leftTopColumn)+"]-r["+
                               QString::number(rightBottomRow)+"]c["+
                               QString::number(rightBottomColumn)+"].", scr_ERROR);
            break;
        }
        break;
     }
     __message("__Excel_slot_Get_range finished;",scr_INFO);
     if(LTcell)
     {
         delete LTcell;
         LTcell=nullptr;
     }
     if(RBcell)
     {
         delete RBcell;
         RBcell = nullptr;
     }
     if(range)
     {
         return range;
     }
     else
     {
         return nullptr;
     }

}
void ExcelApp::Excel_slot_break_tests()
{
     __message("Test sequence interrupted by user;",scr_INFO);
    flags|=EF_BreakActive;
}

void ExcelApp::__connect_toCatch(QAxObject * obj)
{
    connect(obj,SIGNAL(exception(int,const QString &,
                                 const QString &,
                                 const QString &)),
            this,SLOT(__catch_AX(int,const QString &,
                                 const QString &,
                                 const QString &)));
}

void ExcelApp::__disconnect_fromCatch(QAxObject * obj)
{
    disconnect(obj,SIGNAL(exception(int,const QString &,
                                 const QString &,
                                 const QString &)),
            this,SLOT(__catch_AX(int,const QString &,
                                 const QString &,
                                 const QString &)));
}

void ExcelApp::__catch_AX(int code, const QString & source,
                const QString & descript,
                const QString & help)
{
    __message("\t\tMessage from QAxBase: \n"
              "\t\t\tcode: ["+QString::number(code)+"];\n"+
              "\t\t\tsource: "+source+";\n"+
              "\t\t\tdescription: "+descript+";\n"+
              "\t\t\thelp: "+help+";",scr_WARNING);
}
