package cauv.gui;

import java.io.IOException;
import java.util.Vector;

import cauv.Config;
import cauv.auv.AUV;
import cauv.auv.MessageSocket;
import cauv.auv.MessageSocket.ConnectionStateObserver;
import cauv.gui.controllers.PS2ControlHandler;
import cauv.gui.views.CameraFeeds;
import cauv.gui.views.MissionControlView;
import cauv.gui.views.SettingsView;
import cauv.gui.views.TelemetryView;

import com.trolltech.qt.core.Qt;
import com.trolltech.qt.core.Qt.Key;
import com.trolltech.qt.core.Qt.KeyboardModifier;
import com.trolltech.qt.gui.*;

public class Main extends QMainWindow implements ConnectionStateObserver {

	Ui_Main ui = new Ui_Main();
	
	static Main auvGUI;
	
	AUV auv;
	Vector<ScreenView> views = new Vector<ScreenView>();

	public static void main(String[] args) {
		QApplication.initialize(args);
		try {
			Config.load();
		} catch (IOException e) {
			System.err.println("Config error: " + e.getMessage());
		}

		Main gui = new Main();
		
		gui.registerScreen(new CameraFeeds());
		gui.registerScreen(new TelemetryView());
		gui.registerScreen(new MissionControlView());
		gui.registerScreen(new SettingsView());
		
		gui.show();

		QApplication.exec();

		try {
			Config.save();
		} catch (IOException e) {
			System.err.println("Config error: " + e.getMessage());
		}
	}

	public Main() {
		auvGUI = this;
		ui.setupUi(this);
		ui.address.setText(Config.ADDRESS);
		ui.port.setValue(Config.AUV_PORT);
		ui.backButton.hide();
		ui.backButton.clicked.connect(this, "back()");
		ui.connectButton.clicked.connect(this, "connect()");
		Main.trace("Initialisation complete");
		Main.trace("Initialisation complete");
	}

	public Main(QWidget parent) {
		super(parent);
		ui.setupUi(this);
	}

	protected void back(){
		ui.informationStack.setCurrentWidget(ui.page);
		ui.backButton.hide();
	}
	
	protected void connect() {
		ui.address.setEnabled(false);
		ui.port.setEnabled(false);
		ui.connectButton.setEnabled(false);
		ui.errorMessage.setText("Connecting...");
		this.repaint();

		Config.ADDRESS = ui.address.text();
		Config.AUV_PORT = ui.port.value();

		try {
			this.auv = new AUV(Config.ADDRESS, Config.AUV_PORT);
			auv.regsiterConnectionStateObserver(this);
			for(ScreenView v: views){
				v.onConnect(auv);
			}
			
			new PS2ControlHandler(auv);
		} catch (IOException e) {
			ui.errorMessage.setText("Connecting to AUV failed. Sigh.");
			ui.address.setEnabled(true);
			ui.port.setEnabled(true);
		} finally {
			ui.connectButton.setEnabled(true);
		}
	}
	
	public void registerScreen(final ScreenView view) {
		
		Main.trace("Registering view ["+view.getScreenName()+"]");
		views.add(view);
		
		// add the main screen to the stack of information panels
		ui.informationStack.addWidget(view.getScreenWidget());
		
		// set up a mouse call-back on the icon so that when it's clicked
		// the main screen is displayed by moving it to the top of the
		// stack
		QGraphicsView graphics = new QGraphicsView() {
			protected void mouseReleaseEvent(QMouseEvent arg) {
				ui.backButton.show();
				ui.informationStack.setCurrentWidget(view.getScreenWidget());
			}
		};
		
		// add it to the view menu
		QMenu menu = new QMenu(this.ui.menuView);
		menu.setTitle(view.getScreenName());
		this.ui.menuView.addMenu(menu);
		
		
		graphics.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff);
		graphics.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff);

		ScreenIcon icon = new ScreenIcon();
		icon.addWidget(graphics);
		icon.setText(view.getScreenName());
		ui.iconLayout.addWidget(icon, ui.iconLayout.count()/4, ui.iconLayout.count()%4);
		
		QGraphicsScene scene = new QGraphicsScene();
		scene.addItem(view.getIconWidget());
		graphics.setScene(scene);
	};

	public static void message(String message){
		auvGUI.ui.all.append(message);
	}
	
	public static void error(String message){
		auvGUI.ui.errors.append(message);
		String icon = "<img style='vertical-align:baseline' src=\"classpath:cauv/gui/resources/icons/sign_remove.png\" /> ";
		message = "<span style=\" color:#8d162c;\">"+message+"</span>";
		Main.message(icon + message);
	}
	
	public static void warning(String message){
		auvGUI.ui.warnings.append(message);
		String icon = "<img style='vertical-align:baseline' src=\"classpath:cauv/gui/resources/icons/sign_warning.png\" /> ";
		message = "<span style=\" color:#8d5f14;\">"+message+"</span>";
		Main.message(icon + message);
	}
	
	public static void trace(String message){
		auvGUI.ui.traces.append(message);
		String icon = "<img style='vertical-align:baseline' src=\"classpath:cauv/gui/resources/icons/sign_info.png\" /> ";
		message = "<span style=\" color:#005632;\">"+message+"</span>";
		Main.message(icon + message);
	}
	
	@Override
	public void onConnect(MessageSocket connection) {
		ui.connectButton.clicked.disconnect(this, "connect()");
		ui.connectButton.clicked.connect(connection, "disconnect()");
		ui.address.setEnabled(false);
		ui.port.setEnabled(false);
		ui.connectButton.setText("Disconnect");
		ui.errorMessage.setText("");
	}

	@Override
	public void onDisconnect(MessageSocket connection) {
		ui.address.setEnabled(true);
		ui.port.setEnabled(true);
		ui.connectButton.setText("Connect");
	}

	@Override
	protected void keyPressEvent(QKeyEvent arg) {
		if (arg.key() == Key.Key_Escape.value()) {
			if(this.isFullScreen())
				this.showNormal();
			else this.back();
		} else if ((arg.modifiers().isSet(KeyboardModifier.AltModifier))
				&& (arg.key() == Key.Key_Return.value())) {
			this.showFullScreen();
		}
		super.keyPressEvent(arg);
	}

}