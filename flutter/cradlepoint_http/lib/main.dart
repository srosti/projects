import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'package:flutter/material.dart'; import 'package:http/http.dart' as http;

// Globals
String username = 'admin';
String password = '########';
String basicAuth =
    'Basic ' + base64Encode(utf8.encode('$username:$password'));
Map<String, String> headers = {
  "Content-type": "application/json",
  "authorization": basicAuth,
  "Content-Type": "application/x-www-form-urlencoded"
};


class MyHttpOverrides extends HttpOverrides{
  @override
  HttpClient createHttpClient(SecurityContext context){
    return super.createHttpClient(context)
      ..badCertificateCallback = (X509Certificate cert, String host, int port)=> true;
  }
}

Future<DnsMode> getMode() async {
  final response = await http.get(
      'https://10.10.10.1/api/config/dns/mode',
      headers: headers);
  print('getMode response: $response.statusCode');

  if (response.statusCode == 200) {
    print(jsonDecode(response.body));
    return DnsMode.fromJson(jsonDecode(response.body));
  } else {
    throw Exception('Failed to dns mode get');
  }
}

Future<DnsMode> updateDnsMode(String title) async {
  String newMode = 'auto';

  // get the current value
  final response = await http.get(
      'https://10.10.10.1/api/config/dns/mode',
      headers: headers);
  if (response.statusCode != 200) {
    throw Exception('Failed to update DnsMode.');
  }

  // toggle the dns mode
  Map<String, dynamic> json = jsonDecode(response.body);
  if (json['data'] == "auto") {
    newMode = "static";
  }
  else {
    newMode = "auto";
  }
  print("new mode is $newMode");

  // update the router
  final updatedResponse = await http.put(
    'https://10.10.10.1/api/config/dns/',
    headers: headers,
    body : 'data={"mode" : "$newMode"}',
      );
  if (updatedResponse.statusCode != 200) {
    throw Exception('Failed to update DnsMode.');
  }

  // verify the result
  final finalResponse = await http.get(
      'https://10.10.10.1/api/config/dns/mode',
      headers: headers);
  json = jsonDecode(finalResponse.body);
  if (finalResponse.statusCode == 200 && json['data'] == newMode) {
    print("final result $json");
  }
  else {
    print("error: unexpected response");
  }

  return DnsMode.fromJson(json);
}

class DnsMode {
  final int id;
  final String title;

  DnsMode({this.id, this.title});

  factory DnsMode.fromJson(Map<String, dynamic> json) {
    String modeString = "Unknown";
    print("Updating State");
    if (json['data'] == "auto") {
      modeString = "YouTube is enabled";
    }
    if (json['data'] == "static") {
      modeString = "YouTube is disabled";
    }
    return DnsMode(
      id: 0,
      title: modeString,
    );
  }
}

void main() {
  HttpOverrides.global = new MyHttpOverrides();
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  MyApp({Key key}) : super(key: key);

  @override
  _MyAppState createState() {
    return _MyAppState();
  }
}

class _MyAppState extends State<MyApp> {
  final TextEditingController _controller = TextEditingController();
  Future<DnsMode> _futureDnsMode;

  @override
  void initState() {
    super.initState();
    _futureDnsMode = getMode();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'DNS Updater',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: Scaffold(
        appBar: AppBar(
          title: Text('DNS Updater'),
        ),
        body: Container(
          alignment: Alignment.center,
          padding: const EdgeInsets.all(8.0),
          child: FutureBuilder<DnsMode>(
            future: _futureDnsMode,
            builder: (context, snapshot) {
              if (snapshot.connectionState == ConnectionState.done) {
                if (snapshot.hasData) {
                  return Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: <Widget>[
                      Text(snapshot.data.title),
//                      TextField(
//                        controller: _controller,
//                        decoration: InputDecoration(hintText: 'Enter Title'),
//                      ),
                      FlatButton(
                       child: Text('Toggle'),
                       color: Colors.blue,
                       onPressed: () async {
                          setState(() {
                            _futureDnsMode = updateDnsMode(_controller.text);
                          });
                        },
                      ),
                    ],
                  );
                } else if (snapshot.hasError) {
                  return Text("${snapshot.error}");
                }
              }
              return CircularProgressIndicator();
            },
          ),
        ),
      ),
    );
  }
}