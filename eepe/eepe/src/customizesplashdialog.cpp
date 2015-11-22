#include "customizesplashdialog.h"
#include "ui_customizesplashdialog.h"

#include <QtGui>
#include "helpers.h"
#include <QFileDialog>
#include <QMessageBox>



customizeSplashDialog::customizeSplashDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::customizeSplashDialog)
{
    ui->setupUi(this);
}

customizeSplashDialog::~customizeSplashDialog()
{
    delete ui;
}

void customizeSplashDialog::on_loadFromHexButton_clicked()
{
    QString fileName;
    QSettings settings("er9x-eePe", "eePe");
//    quint8 temp[HEX_FILE_SIZE] = {0};

    fileName = QFileDialog::getOpenFileName(this,tr("Open"),settings.value("lastDir").toString(),tr("HEX files (*.hex);;"));
    if(fileName.isEmpty())
    {
        return;
    }

    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());
    QImage image(128, 64, QImage::Format_Mono);
    uchar b[SPLASH_SIZE] = {0};
    if(!getSplashHEX(fileName, (uchar *)&b, this))
    {
        QMessageBox::critical(this, tr("Error"),
                              tr("Error reading file %1").arg(fileName));
        return;
    }
/*
    if(!loadiHEX(this, fileName, (quint8*)&temp, HEX_FILE_SIZE, ""))
    {
        QMessageBox::critical(this, tr("Error"),
                              tr("Error reading file %1").arg(fileName));
        return;
    }

//    QByteArray rawData = QByteArray::fromRawData((const char *)&temp, HEX_FILE_SIZE);
//    int pos = rawData.indexOf(QString(SPLASH_MARKER));

    QByteArray rawData = QByteArray::fromRawData((const char *)&temp, HEX_FILE_SIZE);
    QString mark;
    mark.clear();
    mark.append(SPLASH_MARKER);
    mark.append('\0');
    int pos = rawData.indexOf(mark);

    if(pos<0)
    {
        QMessageBox::information(this, tr("Error"),
                              tr("Error reading image from file"));
        return;
    }

    QImage image(128, 64, QImage::Format_Mono);
//    image.loadFromData((const uchar *)&temp[pos + SPLASH_OFFSET],QImage::Format_MonoLSB);
    uchar b[SPLASH_SIZE] = {0};
    memcpy(&b, (const uchar *)&temp[pos + SPLASH_OFFSET], SPLASH_SIZE);
    */

    for(int y=0; y<SPLASH_HEIGHT; y++)
        for(int x=0; x<SPLASH_WIDTH; x++)
            image.setPixel(x,y,((b[SPLASH_WIDTH*(y/8) + x]) & (1<<(y % 8))) ? 0 : 1  );

    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
}

void customizeSplashDialog::on_loadFromImageButton_clicked()
{
    QString supportedImageFormats;
     for (int formatIndex = 0; formatIndex < QImageReader::supportedImageFormats().count(); formatIndex++) {
         supportedImageFormats += QLatin1String(" *.") + QImageReader::supportedImageFormats()[formatIndex];
     }

    QSettings settings("er9x-eePe", "eePe");
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open Image to load"), settings.value("lastDir").toString(), tr("Images (%1)").arg(supportedImageFormats));

    if (!fileName.isEmpty()) {
        QImage image(fileName);
        if (image.isNull()) {
            QMessageBox::critical(this, tr("Error"),
                                     tr("Cannot load %1.").arg(fileName));
            return;
        }

        ui->imageLabel->setPixmap(QPixmap::fromImage(image.scaled(SPLASH_WIDTH, SPLASH_HEIGHT).convertToFormat(QImage::Format_Mono)));
    }
}

void customizeSplashDialog::on_saveToHexButton_clicked()
{    
    QString fileName;
    QSettings settings("er9x-eePe", "eePe");
//    quint8 temp[HEX_FILE_SIZE] = {0};

    fileName = QFileDialog::getSaveFileName(this,tr("Write to file"),settings.value("lastDir").toString(),tr("HEX files (*.hex);;"),0,QFileDialog::DontConfirmOverwrite);
    if(fileName.isEmpty())
    {
        return;
    }

//    int fileSize = loadiHEX(this, fileName, (quint8*)&temp, HEX_FILE_SIZE, "");

//    if(!fileSize)
//    {
//        QMessageBox::critical(this, tr("Error"),
//                              tr("Error reading file %1").arg(fileName));
//        return;
//    }

//    settings.setValue("lastDir",QFileInfo(fileName).dir().absolutePath());

//    QByteArray rawData = QByteArray::fromRawData((const char *)&temp, HEX_FILE_SIZE);
//    int pos = rawData.indexOf(QString(SPLASH_MARKER));

//    if(pos<0)
//    {
//        QMessageBox::information(this, tr("Error"),
//                              tr("Could not find bitmap to replace in file"));
//        return;
//    }


    QImage image = ui->imageLabel->pixmap()->toImage().scaled(SPLASH_WIDTH, SPLASH_HEIGHT).convertToFormat(QImage::Format_MonoLSB);
    uchar b[SPLASH_SIZE] = {0};
    quint8 * p = image.bits();

    for(int y=0; y<SPLASH_HEIGHT; y++)
        for(int x=0; x<SPLASH_WIDTH; x++)
            b[SPLASH_WIDTH*(y/8) + x] |= ((p[(y*SPLASH_WIDTH + x)/8] & (1<<(x%8))) ? 1 : 0)<<(y % 8);

    if(putSplashHEX(fileName, (uchar *)b, this))
    {
        QMessageBox::information(this, tr("Save To File"),
                                 tr("Successfully updated %1").arg(fileName));
    }
    else
    {
        QMessageBox::critical(this, tr("Error"),
                              tr("Error reading file %1").arg(fileName));
    }

    //    memcpy((uchar *)&temp[pos + SPLASH_OFFSET], &b, SPLASH_SIZE);

//    if(saveiHEX(this, fileName, (quint8*)&temp, fileSize, "", 0))
//    {
//        QMessageBox::information(this, tr("Save To File"),
//                              tr("Successfully updated %1").arg(fileName));
//    }


}

void customizeSplashDialog::on_invertColorButton_clicked()
{
    QImage image = ui->imageLabel->pixmap()->toImage();
    image.invertPixels();
    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
}


