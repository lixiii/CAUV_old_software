/********************************************************************************
** Form generated from reading ui file 'SettingsView.jui'
**
** Created: Mon 28. Jun 17:03:15 2010
**      by: Qt User Interface Compiler version 4.5.2
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

package cauv.gui.views;

import com.trolltech.qt.core.*;
import com.trolltech.qt.gui.*;

public class Ui_SettingsView implements com.trolltech.qt.QUiForm<QWidget>
{
    public QVBoxLayout verticalLayout;
    public QToolBox toolBox;
    public QWidget page_2;
    public QGridLayout gridLayout_3;
    public QSpacerItem horizontalSpacer;
    public QWidget widget;
    public QGridLayout gridLayout;
    public QDoubleSpinBox yaw_Ki;
    public QDoubleSpinBox yaw_Kd;
    public QDoubleSpinBox yaw_scale;
    public QDoubleSpinBox pitch_Ki;
    public QDoubleSpinBox pitch_Kd;
    public QDoubleSpinBox pitch_scale;
    public QDoubleSpinBox depth_Ki;
    public QDoubleSpinBox depth_Kd;
    public QDoubleSpinBox depth_scale;
    public QLabel label_3;
    public QLabel label_4;
    public QLabel label_5;
    public QLabel label_6;
    public QDoubleSpinBox yaw_Kp;
    public QDoubleSpinBox pitch_Kp;
    public QDoubleSpinBox depth_Kp;
    public QLabel label_2;
    public QLabel label_7;
    public QLabel label_8;
    public QSpacerItem horizontalSpacer_2;
    public QSpacerItem verticalSpacer;
    public QSpacerItem verticalSpacer_2;
    public QWidget page_4;
    public QHBoxLayout horizontalLayout_2;
    public QSpacerItem horizontalSpacer_6;
    public QGridLayout gridLayout_2;
    public QDoubleSpinBox foreScale;
    public QLabel label_11;
    public QLabel label_12;
    public QDoubleSpinBox aftScale;
    public QPushButton sendDepth;
    public QSpacerItem verticalSpacer_3;
    public QSpacerItem horizontalSpacer_5;
    public QWidget page_3;
    public QHBoxLayout horizontalLayout_4;
    public QSpacerItem horizontalSpacer_8;
    public QGridLayout gridLayout_4;
    public QLabel label_9;
    public QSpinBox debugLevel;
    public QLabel label_10;
    public QSpacerItem verticalSpacer_4;
    public QCheckBox debugMessaging;
    public QSpacerItem verticalSpacer_5;
    public QSpacerItem horizontalSpacer_7;
    public QWidget page;
    public QHBoxLayout horizontalLayout;
    public QSpacerItem horizontalSpacer_3;
    public QWidget widget_2;
    public QLabel label;
    public QSpinBox gamepadID;
    public QPushButton saveSettingsButton;
    public QSpacerItem horizontalSpacer_4;

    public Ui_SettingsView() { super(); }

    public void setupUi(QWidget SettingsView)
    {
        SettingsView.setObjectName("SettingsView");
        SettingsView.resize(new QSize(568, 354).expandedTo(SettingsView.minimumSizeHint()));
        verticalLayout = new QVBoxLayout(SettingsView);
        verticalLayout.setObjectName("verticalLayout");
        toolBox = new QToolBox(SettingsView);
        toolBox.setObjectName("toolBox");
        page_2 = new QWidget();
        page_2.setObjectName("page_2");
        page_2.setGeometry(new QRect(0, 0, 385, 140));
        gridLayout_3 = new QGridLayout(page_2);
        gridLayout_3.setObjectName("gridLayout_3");
        horizontalSpacer = new QSpacerItem(40, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        gridLayout_3.addItem(horizontalSpacer, 1, 0, 1, 1);

        widget = new QWidget(page_2);
        widget.setObjectName("widget");
        gridLayout = new QGridLayout(widget);
        gridLayout.setObjectName("gridLayout");
        yaw_Ki = new QDoubleSpinBox(widget);
        yaw_Ki.setObjectName("yaw_Ki");
        yaw_Ki.setMinimum(-10000);
        yaw_Ki.setMaximum(10000);
        yaw_Ki.setSingleStep(0.01);

        gridLayout.addWidget(yaw_Ki, 2, 3, 1, 1);

        yaw_Kd = new QDoubleSpinBox(widget);
        yaw_Kd.setObjectName("yaw_Kd");
        yaw_Kd.setMinimum(-10000);
        yaw_Kd.setMaximum(10000);
        yaw_Kd.setSingleStep(0.01);

        gridLayout.addWidget(yaw_Kd, 2, 4, 1, 1);

        yaw_scale = new QDoubleSpinBox(widget);
        yaw_scale.setObjectName("yaw_scale");
        yaw_scale.setMinimum(-10000);
        yaw_scale.setMaximum(10000);
        yaw_scale.setSingleStep(0.01);

        gridLayout.addWidget(yaw_scale, 2, 5, 1, 1);

        pitch_Ki = new QDoubleSpinBox(widget);
        pitch_Ki.setObjectName("pitch_Ki");
        pitch_Ki.setMinimum(-10000);
        pitch_Ki.setMaximum(10000);
        pitch_Ki.setSingleStep(0.01);

        gridLayout.addWidget(pitch_Ki, 6, 3, 1, 1);

        pitch_Kd = new QDoubleSpinBox(widget);
        pitch_Kd.setObjectName("pitch_Kd");
        pitch_Kd.setMinimum(-10000);
        pitch_Kd.setMaximum(10000);
        pitch_Kd.setSingleStep(0.01);

        gridLayout.addWidget(pitch_Kd, 6, 4, 1, 1);

        pitch_scale = new QDoubleSpinBox(widget);
        pitch_scale.setObjectName("pitch_scale");
        pitch_scale.setMinimum(-10000);
        pitch_scale.setMaximum(10000);
        pitch_scale.setSingleStep(0.01);

        gridLayout.addWidget(pitch_scale, 6, 5, 1, 1);

        depth_Ki = new QDoubleSpinBox(widget);
        depth_Ki.setObjectName("depth_Ki");
        depth_Ki.setMinimum(-10000);
        depth_Ki.setMaximum(10000);
        depth_Ki.setSingleStep(0.01);

        gridLayout.addWidget(depth_Ki, 7, 3, 1, 1);

        depth_Kd = new QDoubleSpinBox(widget);
        depth_Kd.setObjectName("depth_Kd");
        depth_Kd.setMinimum(-10000);
        depth_Kd.setMaximum(10000);
        depth_Kd.setSingleStep(0.01);

        gridLayout.addWidget(depth_Kd, 7, 4, 1, 1);

        depth_scale = new QDoubleSpinBox(widget);
        depth_scale.setObjectName("depth_scale");
        depth_scale.setMinimum(-10000);
        depth_scale.setMaximum(10000);
        depth_scale.setSingleStep(0.01);

        gridLayout.addWidget(depth_scale, 7, 5, 1, 1);

        label_3 = new QLabel(widget);
        label_3.setObjectName("label_3");
        label_3.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_3, 1, 3, 1, 1);

        label_4 = new QLabel(widget);
        label_4.setObjectName("label_4");
        label_4.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_4, 1, 4, 1, 1);

        label_5 = new QLabel(widget);
        label_5.setObjectName("label_5");
        label_5.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_5, 1, 5, 1, 1);

        label_6 = new QLabel(widget);
        label_6.setObjectName("label_6");
        label_6.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_6, 2, 1, 1, 1);

        yaw_Kp = new QDoubleSpinBox(widget);
        yaw_Kp.setObjectName("yaw_Kp");
        yaw_Kp.setMinimum(-10000);
        yaw_Kp.setMaximum(10000);
        yaw_Kp.setSingleStep(0.01);

        gridLayout.addWidget(yaw_Kp, 2, 2, 1, 1);

        pitch_Kp = new QDoubleSpinBox(widget);
        pitch_Kp.setObjectName("pitch_Kp");
        pitch_Kp.setMinimum(-10000);
        pitch_Kp.setMaximum(10000);
        pitch_Kp.setSingleStep(0.01);

        gridLayout.addWidget(pitch_Kp, 6, 2, 1, 1);

        depth_Kp = new QDoubleSpinBox(widget);
        depth_Kp.setObjectName("depth_Kp");
        depth_Kp.setMinimum(-10000);
        depth_Kp.setMaximum(10000);
        depth_Kp.setSingleStep(0.01);

        gridLayout.addWidget(depth_Kp, 7, 2, 1, 1);

        label_2 = new QLabel(widget);
        label_2.setObjectName("label_2");
        label_2.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_2, 1, 2, 1, 1);

        label_7 = new QLabel(widget);
        label_7.setObjectName("label_7");
        label_7.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_7, 6, 1, 1, 1);

        label_8 = new QLabel(widget);
        label_8.setObjectName("label_8");
        label_8.setAlignment(com.trolltech.qt.core.Qt.AlignmentFlag.createQFlags(com.trolltech.qt.core.Qt.AlignmentFlag.AlignCenter));

        gridLayout.addWidget(label_8, 7, 1, 1, 1);


        gridLayout_3.addWidget(widget, 1, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        gridLayout_3.addItem(horizontalSpacer_2, 1, 2, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding);

        gridLayout_3.addItem(verticalSpacer, 0, 1, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 40, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding);

        gridLayout_3.addItem(verticalSpacer_2, 2, 1, 1, 1);

        toolBox.addItem(page_2, com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Autopilot Settings", null));
        page_4 = new QWidget();
        page_4.setObjectName("page_4");
        page_4.setGeometry(new QRect(0, 0, 550, 224));
        horizontalLayout_2 = new QHBoxLayout(page_4);
        horizontalLayout_2.setObjectName("horizontalLayout_2");
        horizontalSpacer_6 = new QSpacerItem(143, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        horizontalLayout_2.addItem(horizontalSpacer_6);

        gridLayout_2 = new QGridLayout();
        gridLayout_2.setObjectName("gridLayout_2");
        foreScale = new QDoubleSpinBox(page_4);
        foreScale.setObjectName("foreScale");
        foreScale.setMinimum(-10000);
        foreScale.setMaximum(1000);
        foreScale.setSingleStep(0.01);

        gridLayout_2.addWidget(foreScale, 0, 1, 1, 1);

        label_11 = new QLabel(page_4);
        label_11.setObjectName("label_11");

        gridLayout_2.addWidget(label_11, 0, 0, 1, 1);

        label_12 = new QLabel(page_4);
        label_12.setObjectName("label_12");

        gridLayout_2.addWidget(label_12, 1, 0, 1, 1);

        aftScale = new QDoubleSpinBox(page_4);
        aftScale.setObjectName("aftScale");
        aftScale.setMinimum(-10000);
        aftScale.setMaximum(1000);
        aftScale.setSingleStep(0.01);

        gridLayout_2.addWidget(aftScale, 1, 1, 1, 1);

        sendDepth = new QPushButton(page_4);
        sendDepth.setObjectName("sendDepth");

        gridLayout_2.addWidget(sendDepth, 2, 1, 1, 1);

        verticalSpacer_3 = new QSpacerItem(20, 40, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding);

        gridLayout_2.addItem(verticalSpacer_3, 3, 1, 1, 1);


        horizontalLayout_2.addLayout(gridLayout_2);

        horizontalSpacer_5 = new QSpacerItem(142, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        horizontalLayout_2.addItem(horizontalSpacer_5);

        toolBox.addItem(page_4, com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Depth Calibration", null));
        page_3 = new QWidget();
        page_3.setObjectName("page_3");
        page_3.setGeometry(new QRect(0, 0, 550, 224));
        horizontalLayout_4 = new QHBoxLayout(page_3);
        horizontalLayout_4.setObjectName("horizontalLayout_4");
        horizontalSpacer_8 = new QSpacerItem(184, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        horizontalLayout_4.addItem(horizontalSpacer_8);

        gridLayout_4 = new QGridLayout();
        gridLayout_4.setObjectName("gridLayout_4");
        label_9 = new QLabel(page_3);
        label_9.setObjectName("label_9");

        gridLayout_4.addWidget(label_9, 1, 0, 1, 1);

        debugLevel = new QSpinBox(page_3);
        debugLevel.setObjectName("debugLevel");
        QSizePolicy sizePolicy = new QSizePolicy(com.trolltech.qt.gui.QSizePolicy.Policy.Fixed, com.trolltech.qt.gui.QSizePolicy.Policy.Fixed);
        sizePolicy.setHorizontalStretch((byte)0);
        sizePolicy.setVerticalStretch((byte)0);
        sizePolicy.setHeightForWidth(debugLevel.sizePolicy().hasHeightForWidth());
        debugLevel.setSizePolicy(sizePolicy);
        debugLevel.setMinimum(-10000);
        debugLevel.setMaximum(10000);
        debugLevel.setValue(1);

        gridLayout_4.addWidget(debugLevel, 1, 1, 1, 1);

        label_10 = new QLabel(page_3);
        label_10.setObjectName("label_10");

        gridLayout_4.addWidget(label_10, 2, 0, 1, 1);

        verticalSpacer_4 = new QSpacerItem(20, 40, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding);

        gridLayout_4.addItem(verticalSpacer_4, 5, 1, 1, 1);

        debugMessaging = new QCheckBox(page_3);
        debugMessaging.setObjectName("debugMessaging");

        gridLayout_4.addWidget(debugMessaging, 2, 1, 1, 1);

        verticalSpacer_5 = new QSpacerItem(20, 40, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding);

        gridLayout_4.addItem(verticalSpacer_5, 0, 1, 1, 1);


        horizontalLayout_4.addLayout(gridLayout_4);

        horizontalSpacer_7 = new QSpacerItem(183, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        horizontalLayout_4.addItem(horizontalSpacer_7);

        toolBox.addItem(page_3, com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Debug Settings", null));
        page = new QWidget();
        page.setObjectName("page");
        page.setGeometry(new QRect(0, 0, 230, 38));
        horizontalLayout = new QHBoxLayout(page);
        horizontalLayout.setObjectName("horizontalLayout");
        horizontalSpacer_3 = new QSpacerItem(40, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        horizontalLayout.addItem(horizontalSpacer_3);

        widget_2 = new QWidget(page);
        widget_2.setObjectName("widget_2");
        widget_2.setMinimumSize(new QSize(200, 0));
        label = new QLabel(widget_2);
        label.setObjectName("label");
        label.setGeometry(new QRect(20, 20, 59, 21));
        gamepadID = new QSpinBox(widget_2);
        gamepadID.setObjectName("gamepadID");
        gamepadID.setGeometry(new QRect(90, 20, 61, 20));
        QPalette palette= new QPalette();
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.WindowText, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Button, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Light, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Midlight, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Dark, new QColor(127, 127, 127));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Mid, new QColor(170, 170, 170));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Text, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.BrightText, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.ButtonText, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Base, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Window, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Shadow, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.AlternateBase, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.ToolTipBase, new QColor(255, 255, 220));
        palette.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.ToolTipText, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.WindowText, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Button, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Light, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Midlight, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Dark, new QColor(127, 127, 127));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Mid, new QColor(170, 170, 170));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Text, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.BrightText, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.ButtonText, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Base, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Window, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Shadow, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.AlternateBase, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.ToolTipBase, new QColor(255, 255, 220));
        palette.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.ToolTipText, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.WindowText, new QColor(127, 127, 127));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Button, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Light, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Midlight, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Dark, new QColor(127, 127, 127));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Mid, new QColor(170, 170, 170));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Text, new QColor(127, 127, 127));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.BrightText, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.ButtonText, new QColor(127, 127, 127));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Base, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Window, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Shadow, new QColor(0, 0, 0));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.AlternateBase, new QColor(255, 255, 255));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.ToolTipBase, new QColor(255, 255, 220));
        palette.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.ToolTipText, new QColor(0, 0, 0));
        gamepadID.setPalette(palette);
        saveSettingsButton = new QPushButton(widget_2);
        saveSettingsButton.setObjectName("saveSettingsButton");
        saveSettingsButton.setGeometry(new QRect(80, 50, 71, 23));
        QPalette palette1= new QPalette();
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.WindowText, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Button, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Light, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Midlight, new QColor(243, 243, 243));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Dark, new QColor(116, 116, 116));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Mid, new QColor(155, 155, 155));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Text, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.BrightText, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.ButtonText, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Base, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Window, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.Shadow, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.AlternateBase, new QColor(243, 243, 243));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.ToolTipBase, new QColor(255, 255, 220));
        palette1.setColor(QPalette.ColorGroup.Active, QPalette.ColorRole.ToolTipText, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.WindowText, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Button, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Light, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Midlight, new QColor(243, 243, 243));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Dark, new QColor(116, 116, 116));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Mid, new QColor(155, 155, 155));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Text, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.BrightText, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.ButtonText, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Base, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Window, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.Shadow, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.AlternateBase, new QColor(243, 243, 243));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.ToolTipBase, new QColor(255, 255, 220));
        palette1.setColor(QPalette.ColorGroup.Inactive, QPalette.ColorRole.ToolTipText, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.WindowText, new QColor(116, 116, 116));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Button, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Light, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Midlight, new QColor(243, 243, 243));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Dark, new QColor(116, 116, 116));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Mid, new QColor(155, 155, 155));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Text, new QColor(116, 116, 116));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.BrightText, new QColor(255, 255, 255));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.ButtonText, new QColor(116, 116, 116));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Base, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Window, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.Shadow, new QColor(0, 0, 0));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.AlternateBase, new QColor(232, 232, 232));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.ToolTipBase, new QColor(255, 255, 220));
        palette1.setColor(QPalette.ColorGroup.Disabled, QPalette.ColorRole.ToolTipText, new QColor(0, 0, 0));
        saveSettingsButton.setPalette(palette1);

        horizontalLayout.addWidget(widget_2);

        horizontalSpacer_4 = new QSpacerItem(40, 20, com.trolltech.qt.gui.QSizePolicy.Policy.Expanding, com.trolltech.qt.gui.QSizePolicy.Policy.Minimum);

        horizontalLayout.addItem(horizontalSpacer_4);

        toolBox.addItem(page, com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Gamepad Settings", null));

        verticalLayout.addWidget(toolBox);

        retranslateUi(SettingsView);

        toolBox.setCurrentIndex(1);


        SettingsView.connectSlotsByName();
    } // setupUi

    void retranslateUi(QWidget SettingsView)
    {
        SettingsView.setWindowTitle(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Form", null));
        label_3.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Ki", null));
        label_4.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Kd", null));
        label_5.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "scale", null));
        label_6.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Yaw", null));
        label_2.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Kp", null));
        label_7.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Pitch", null));
        label_8.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Depth", null));
        toolBox.setItemText(toolBox.indexOf(page_2), com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Autopilot Settings", null));
        label_11.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Fore Scale", null));
        label_12.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Aft Scale", null));
        sendDepth.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Send", null));
        toolBox.setItemText(toolBox.indexOf(page_4), com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Depth Calibration", null));
        label_9.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Debug Level ", null));
        label_10.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Debug Messaging", null));
        debugMessaging.setText("");
        toolBox.setItemText(toolBox.indexOf(page_3), com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Debug Settings", null));
        label.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Gamepad ID", null));
        saveSettingsButton.setText(com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Save", null));
        toolBox.setItemText(toolBox.indexOf(page), com.trolltech.qt.core.QCoreApplication.translate("SettingsView", "Gamepad Settings", null));
    } // retranslateUi

}

