#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <TridentTD_LineNotify.h>

#define MQ2_PIN 34  // MQ2(CO感測器) => GPIO 34
#define GREEN_LED_PIN 12  // 綠色LED => GPIO 12
#define RED_LED_PIN 14    // 紅色LED => GPIO 14
#define BUZZER_PIN 27     // 蜂鳴器 => GPIO 27
#define INIT_DELAY_SECOND 60  // 初始等待秒數(default=60)
#define THRESHOLD 900    // MQ2數值門檻值(default=900)
#define CONTINUE_NUMBER 10 // 連續高於or低於次數(default=10)

char SSID[] = "<你的WiFi名稱>";  // WiFi名稱
char PASSWORD[] = "<你的WiFi密碼>";   // WiFi密碼
String LINE_TOKEN = "<你的LINE notify token>";  // LINE notify token

int higher_counter = 0; // 紀錄低於次數
int lower_counter = 0;  // 記錄高於次數
int state = 0;  // 警報狀態(0關、1開)

void setup() {
  // 初始設定
  Serial.begin(115200);
  pinMode(MQ2_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  LINE.setToken(LINE_TOKEN);  // 設定LINE notify token

  Serial.println("\n啟動一氧化碳中毒警示器!");

  //連線到指定的WiFi SSID
  Serial.print("Connecting Wifi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //連線成功，顯示取得的IP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  // 需加熱1分鐘後才可正確量測
  Serial.println("請先等1分鐘後才可開始正確量測...");
  for(int i = INIT_DELAY_SECOND; i > 0; i--) {
    Serial.print("剩餘 "); Serial.print(i); Serial.println(" 秒");
    delay(1000);
  }
  Serial.println("開始監測一氧化碳濃度~");
  LINE.notifySticker("開始監測一氧化碳濃度~", 11538, 51626494); // LINE傳送貼圖
}

void loop() {
  int MQ2_value = analogRead(MQ2_PIN);  // 讀取MQ2的數值
  Serial.print("MQ2 = "); Serial.println(MQ2_value);
  if(MQ2_value > THRESHOLD) { // 超過門檻值
    higher_counter++;
    lower_counter = 0;
    if(higher_counter == CONTINUE_NUMBER && state == 0) { // 連續10次超過門檻且警示關閉
      Serial.println("開啟警示!");
      state = 1;  // 開啟警示
      line_notify_danger(MQ2_value);
    }
  }
  else {  // 低於門檻值
    lower_counter++;
    higher_counter = 0;
    if(lower_counter == CONTINUE_NUMBER && state == 1) { // 連續10次低於門檻且警示開啟
      Serial.println("關閉警示!");
      state = 0;  // 關閉警示
      line_notify_safe(MQ2_value);
    }
  }

  if(state == 0) {  // 警示關閉狀態
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  else {  // 警示開啟狀態
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  
  delay(1000);
}

// LINE notify通知警報
void line_notify_danger(int value) {
  String msg = "警告!!!\n偵測到一氧化碳濃度超標的情況，請盡速聯繫同住家人或朋友。\nvalue = " + String(value);
//  Serial.println(msg);
  LINE.notify(msg);
  LINE.notifyPicture("\n一氧化碳中毒處置措施：\n1. 立即停止使用熱水器，打開窗戶通風。\n2. 前往室外避難，撥打119電話求助。\n3. 患者如無呼吸心跳，請依急救步驟實施心肺復甦術(CPR)。", "https://i.imgur.com/vlWBo9p.jpg");
}

// LINE notify通知警報解除
void line_notify_safe(int value) {
  String msg = "警告解除。\n有可能是感測器故障所致，請向同住家人或朋友確認。\nvalue = " + String(value);
//  Serial.println(msg);
  LINE.notify(msg);
}
