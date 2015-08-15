/*======================================================
    > File Name: UDFileWidget.h
    > Author: lyh
    > E-mail:  
    > Other :  
    > Created Time: 2015年08月11日 星期二 14时53分45秒
 =======================================================*/

#include"udfilewidget.h"
#include"uditemwidget.h"
#include"connectserv.h"

#include<QHBoxLayout>
#include<QLabel>
#include<QListWidgetItem>
#include<QWidget>
#include<QProgressBar>
#include<QPushButton>
#include<QListWidgetItem>

#include<json/json.h>

UDFileWidget::UDFileWidget(QTabWidget* parent):QTabWidget(parent)
{
    setStyleSheet("QTabBar::tab{background: #D1EEEE;min-width:135;min-height:40;} \ QTabBar::tab:hover{background: white} \ QTabBar::tab:selected{background:white;border-color: #EE0000}");

    upListWidget = new QListWidget;
//    upListWidget->setFrameShape(QListWidget::NoFrame);
    downListWidget = new QListWidget;

    QWidget*     upitemWidget = new QWidget(this);
    QWidget*     dwitemWidget = new QWidget(this);
    QHBoxLayout* upitemLayout = new QHBoxLayout;
    QHBoxLayout* dwitemLayout = new QHBoxLayout;
    QLabel*      upstateLabel = new QLabel(tr("状态"));
    upstateLabel->setStyleSheet("max-width:80;");
    upstateLabel->setAlignment(Qt::AlignCenter);
    QLabel*      dwstateLabel = new QLabel(tr("状态"));
    dwstateLabel->setStyleSheet("max-width:80;");
    dwstateLabel->setAlignment(Qt::AlignCenter);

    QLabel*      upfileLabel = new QLabel(tr("上传到"));
    upfileLabel->setAlignment(Qt::AlignCenter);
    QLabel*      dwfileLabel = new QLabel(tr("文件"));
    dwfileLabel->setAlignment(Qt::AlignCenter);
    QLabel*      upproLabel = new QLabel(tr("进度"));
    upproLabel->setAlignment(Qt::AlignCenter);
    QLabel*      dwproLabel = new QLabel(tr("进度"));
    dwproLabel->setAlignment(Qt::AlignCenter);

    QLabel*      upoptionLabel = new QLabel(tr("操作"));
    upoptionLabel->setStyleSheet("max-width:140;");
    upoptionLabel->setAlignment(Qt::AlignCenter);
    QLabel*      dwoptionLabel = new QLabel(tr("操作"));
    dwoptionLabel->setStyleSheet("max-width:140;");
    dwoptionLabel->setAlignment(Qt::AlignCenter);
 
    upitemLayout->addWidget(upstateLabel);
    upitemLayout->addWidget(upfileLabel);
    upitemLayout->addWidget(upproLabel);
    upitemLayout->addWidget(upoptionLabel);
    addTab(upListWidget,tr("上传列表"));
    upitemWidget->setLayout(upitemLayout);
    upitemWidget->setStyleSheet("background:#87CEFF");

    QListWidgetItem*  upitem = new QListWidgetItem;
    upitem->setSizeHint(QSize(0,40));
    upitem->setFlags(Qt::NoItemFlags);
    upListWidget->addItem(upitem);
    upListWidget->setItemWidget(upitem,upitemWidget);

    addTab(downListWidget,tr("下载列表"));
    dwitemLayout->addWidget(dwstateLabel);
    dwitemLayout->addWidget(dwfileLabel);
    dwitemLayout->addWidget(dwproLabel);
    dwitemLayout->addWidget(dwoptionLabel);
    dwitemWidget->setLayout(dwitemLayout);
    dwitemWidget->setStyleSheet("background: #87CEFF");
    QListWidgetItem*  dwitem = new QListWidgetItem;
    downListWidget->addItem(dwitem);
    dwitem->setSizeHint(QSize(0,40));
    dwitem->setFlags(Qt::NoItemFlags);
    downListWidget->setItemWidget(dwitem,dwitemWidget);

    qRegisterMetaType<string>("string");
    connect(this,SIGNAL(UpFileSig(string)),this,SLOT(UpFile(string)));
    connect(this,SIGNAL(DownFileSig(string)),this,SLOT(DownFile(string)));
}

//发射上传文件信号
void UDFileWidget::EmitUpFileSig(string json)
{
    cout<<"emit up\n"<<json<<endl;
    emit UpFileSig(json);
}

//发射下载文件信号
void UDFileWidget::EmitDownFileSig(string json)
{
    emit DownFileSig(json);
}

//上传文件
void UDFileWidget::UpFile(string strjson)
{
    string         strfile;
    Json::Value    json;
    Json::Reader   reader;
    if(reader.parse(strjson,json))
    {
        strfile = json["File"].asString();
    }

    //添加条目
    UDItemWidget*    iWidget = new UDItemWidget(2,this);
    QListWidgetItem* item = new QListWidgetItem;

    iWidget->setFileLabel(QString::fromStdString(strfile));
    item->setSizeHint(QSize(0,40));
    this->upListWidget->addItem(item);
    this->upListWidget->setItemWidget(item,iWidget);

    //添加线程参数
    struct Param*        arg = new struct Param;
    arg->p = this;
    arg->strjson = strjson;
    arg->item = item;
   
    pthread_t           thid;
    pthread_create(&thid,NULL,UpFileThread,(void*)arg);
}

//下载文件
void UDFileWidget::DownFile(string strjson)
{
    string   strfile,strpath,strsave;
    //解析Json
    Json::Value      json;
    Json::Reader     reader;
    if(reader.parse(strjson,json))
    {
        strpath = json["Path"].asString();
        strsave = json["SavePath"].asString();
    }

    //添加条目
    UDItemWidget*  iWidget = new UDItemWidget(1,this);
    QListWidgetItem* item = new QListWidgetItem;

    iWidget->setFileLabel(QString::fromStdString(strpath));
    item->setSizeHint(QSize(0,40));
    this->downListWidget->addItem(item);
    this->downListWidget->setItemWidget(item,iWidget);
    //添加线程参数
    struct Param*    arg = new struct Param;
    arg->p = this;
    arg->strjson = strjson;
    arg->item = item;

    pthread_t        thid;
    pthread_create(&thid,NULL,DownFileThread,(void*)arg);
}

/* 上传文件线程 */
void* UDFileWidget::UpFileThread(void* arg)
{
    cout<<"UpFile\n";

    UDFileWidget*  cthis = ((struct Param*)arg)->p;
    string    strjson = ((struct Param*)arg)->strjson;  
    QListWidgetItem* item = ((struct Param*)arg)->item;
    UDItemWidget*  iWidget = (UDItemWidget*)(cthis->upListWidget->itemWidget(item));

    cout<<"upfile\n"<<strjson<<endl;
    string          strip,strport,strfile;
    Json::Reader    reader;
    Json::Value     json;
    if(reader.parse(strjson,json))
    {
        strip = json["Ip"].asString();
        strport = json["Port"].asString();
        strfile = json["File"].asString();
    }
    
    //连接子服务器
    int             worksockFd;
    int  ret = ConnectServer(QString::fromStdString(strip),QString::fromStdString(strport));
    if(ret<0)
    {
        iWidget->EmitStateSig(tr("连接失败"));
        cout<<"连接失败\n";
        return NULL;
    }
    worksockFd = ret;
    //获取Md5
    char     md5buf[33];
    memset(md5buf,'\0',sizeof(md5buf));
    FILE*    fp;
    string   md5sum;
    md5sum = "md5sum "+strfile;
    fp = popen(md5sum.c_str(),"r");
    fread(md5buf,sizeof(char),sizeof(md5buf),fp);
    pclose(fp);
    md5buf[32] = '\0';

    //发送Md5文件名
    Json::Value       upjson;
    upjson["mark"] = Json::Value(2);
    upjson["md5"] = Json::Value(md5buf);
    string        buf;
    buf = upjson.toStyledString();
    if(send(worksockFd,buf.c_str(),strlen(buf.c_str()),0)<0)
    {
        cout<<"上传失败\n";
        return NULL;
    }
    int       fileFd;
    if((fileFd = openfile(strfile.c_str(),O_RDONLY))==-1)
    {
        cout<<"文件不存在\n";
        return NULL;
    }
    struct stat  statFile;
    fstat(fileFd,&statFile);  
    int     sum=0;
    ssize_t     len = 0;
    off_t       pos = lseek(fileFd,0,SEEK_SET);
    if(pos<0)
    {
        cout<<"上传失败\n";
        return NULL;
    }
    iWidget->EmitProBarRange(0,statFile.st_size);
    while(1)
    {
        len = sendfile(worksockFd,fileFd,&pos,statFile.st_size);
        
        if(len<=0)
            break;
        sum = sum+len;
        iWidget->EmitProBarValue(sum);
        cout<<"sum="<<sum<<endl;
    }
    cout<<"statFile.st_size = "<<statFile.st_size<<endl;
    cout<<"len = "<<len<<endl;
    closefd(fileFd);

    cout<<"sum = "<<sum<<endl;

    if(len == -1)
    {
        cout<<"使用sendfile失败"<<endl;
        return NULL;
    }

    if(sum >= statFile.st_size)
    {
        iWidget->EmitStateSig(tr("上传完成"));
        cout<<"文件上传成功"<<endl;
        pthread_exit(0);
        return NULL;
    }
    return NULL;
}

//下载文件线程
void*  UDFileWidget::DownFileThread(void* arg)
{
    UDFileWidget*   cthis = ((struct Param*)arg)->p;
    string strjson = ((struct Param*)arg)->strjson;
    QListWidgetItem* item = ((struct Param*)arg)->item;
    UDItemWidget* iWidget = (UDItemWidget*)(cthis->downListWidget->itemWidget(item));

    string  strip,strport;
    string  strfile; //下载文件名
    string  strpath; //下载文件路径
    string  strsave; //文件保存路径
    string  strfiles;//服务器文件存储路径
    string  strmd5;  //文件Md5
    int     filesize = 0;//文件大小

    //解析Json
    Json::Reader    reader;
    Json::Value     json;
    if(reader.parse(strjson,json))
    {
        /*获取子服务器ip*/
        //返回‘/’第一次出现的位置
        string::size_type position;
        strfiles = json["Files"].asString();
        position = strfiles.find_first_of("/");
        strip = strfiles.substr(0,position);

        strport = json["Port"].asString();
        strsave = json["SavePath"].asString();
        strmd5 = json["Md5"].asString();
        strfile = json["File"].asString();
        filesize = json["Size"].asInt();
    }

    cout<<"ip: "<<strip<<"port: "<<strport<<endl;
    //连接子服务器
    int        worksockFd;
    int  ret = ConnectServer(QString::fromStdString(strip),QString::fromStdString(strport));
    if(ret<0)
    {
        iWidget->EmitStateSig(tr("连接失败"));
        cout<<"连接失败\n";
        return NULL;
    }
    worksockFd = ret;
    //给子服务器发送下载请求
    Json::Value     downjson;
    downjson["mark"] = Json::Value(1);
    downjson["md5"] = Json::Value(strmd5);
    string      buf;
    buf = downjson.toStyledString();
    if(send(worksockFd,buf.c_str(),strlen(buf.c_str()),0)<0)
    {
        iWidget->EmitStateSig(tr("下载失败"));
        cout<<"下载失败\n";
        return NULL;
    }

    iWidget->EmitProBarRange(0,filesize);

    //接收文件
    int    fileFd;
    cout<<"savePath:"<<strsave<<endl;
    string     strsavefile = strsave+"/"+strfile;
    if((fileFd = openfile(strsavefile.c_str()))<0)
    {
        cout<<"文件保存失败\n";
        iWidget->EmitStateSig(tr("保存失败"));
        return NULL;
    }

    //使用splice拷贝文件
    int pipeFd[2];

    //创建管道
    ret = pipe(pipeFd);
    if(ret < 0)
    {
        cout<<"创建管道失败\n";
        iWidget->EmitStateSig(tr("下载失败\n"));
        return NULL;
    }

    int sum = 0;

    off_t lenth = 0;
    //从发送端套接字将文件读入到管道
    while(1)
    {
        cout<<"splice...\n";

        ret = splice(worksockFd,NULL,pipeFd[1],NULL,32768,SPLICE_F_MOVE);
        sum = sum + ret;
     
        cout<<"ret11 = "<<ret<<endl;
        //从管道读出内容写入文件
        ret = splice(pipeFd[0],NULL,fileFd,NULL,32768,SPLICE_F_MOVE);
        lenth  = lenth + ret;
        cout<<"ret22 = "<<ret<<endl;
        iWidget->EmitProBarValue(lenth);
        if(ret <= 0)
        {
            break;
        }
    }
    cout<<"splice end\n";

    if(ret == -1)
    {
        cout<<"下载出错\n";
        iWidget->EmitStateSig(tr("下载失败"));
        closefd(fileFd);
        return NULL;
    }

    cout<<"sum = "<<sum<<endl;
    cout<<"下载完成\n";
    iWidget->EmitStateSig(tr("下载完成"));
    closefd(fileFd);
    return NULL;
}
