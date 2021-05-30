#include "assets_win.h"

#include "picojson.h"

struct t_status_code {
	int code;
	PCWCHAR status;
};

// based on https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
static const struct t_status_code status_codes[] = {
	{100, L"Continue"}, // [RFC7231, Section 6.2.1]
	{101, L"Switching Protocols"}, // [RFC7231, Section 6.2.2]
	{102, L"Processing"}, // [RFC2518]
	{103, L"Early Hints"}, // [RFC8297]
	// 104 - 199 Unassigned
	{200, L"OK"}, // [RFC7231, Section 6.3.1]
	{201, L"Created"}, // [RFC7231, Section 6.3.2]
	{202, L"Accepted"}, // [RFC7231, Section 6.3.3]
	{203, L"Non - Authoritative Information"}, // [RFC7231, Section 6.3.4]
	{204, L"No Content"}, // [RFC7231, Section 6.3.5]
	{205, L"Reset Content"}, // [RFC7231, Section 6.3.6]
	{206, L"Partial Content"}, // [RFC7233, Section 4.1]
	{207, L"Multi - Status"}, // [RFC4918]
	{208, L"Already Reported"}, // [RFC5842]
	// 209 - 225 Unassigned
	{226, L"IM Used"}, // [RFC3229]
	// 227 - 299 Unassigned
	{300, L"Multiple Choices"}, // [RFC7231, Section 6.4.1]
	{301, L"Moved Permanently"}, // [RFC7231, Section 6.4.2]
	{302, L"Found"}, // [RFC7231, Section 6.4.3]
	{303, L"See Other"}, // [RFC7231, Section 6.4.4]
	{304, L"Not Modified"}, // [RFC7232, Section 4.1]
	{305, L"Use Proxy"}, // [RFC7231, Section 6.4.5]
	{306, L"(Unused)"}, // [RFC7231, Section 6.4.6]
	{307, L"Temporary Redirect"}, // [RFC7231, Section 6.4.7]
	{308, L"Permanent Redirect"}, // [RFC7538]
	// 309 - 399 Unassigned
	{400, L"Bad Request"}, // [RFC7231, Section 6.5.1]
	{401, L"Unauthorized"}, // [RFC7235, Section 3.1]
	{402, L"Payment Required"}, // [RFC7231, Section 6.5.2]
	{403, L"Forbidden"}, // [RFC7231, Section 6.5.3]
	{404, L"Not Found"}, // [RFC7231, Section 6.5.4]
	{405, L"Method Not Allowed"}, // [RFC7231, Section 6.5.5]
	{406, L"Not Acceptable"}, // [RFC7231, Section 6.5.6]
	{407, L"Proxy Authentication Required"}, // [RFC7235, Section 3.2]
	{408, L"Request Timeout"}, // [RFC7231, Section 6.5.7]
	{409, L"Conflict"}, // [RFC7231, Section 6.5.8]
	{410, L"Gone"}, // [RFC7231, Section 6.5.9]
	{411, L"Length Required"}, // [RFC7231, Section 6.5.10]
	{412, L"Precondition Failed"}, // [RFC7232, Section 4.2]"}, // [RFC8144, Section 3.2]
	{413, L"Payload Too Large"}, // [RFC7231, Section 6.5.11]
	{414, L"URI Too Long"}, // [RFC7231, Section 6.5.12]
	{415, L"Unsupported Media Type"}, // [RFC7231, Section 6.5.13]"}, // [RFC7694, Section 3]
	{416, L"Range Not Satisfiable"}, // [RFC7233, Section 4.4]
	{417, L"Expectation Failed"}, // [RFC7231, Section 6.5.14]
	// 418 - 420 Unassigned
	{421, L"Misdirected Request"}, // [RFC7540, Section 9.1.2]
	{422, L"Unprocessable Entity"}, // [RFC4918]
	{423, L"Locked"}, // [RFC4918]
	{424, L"Failed Dependency"}, // [RFC4918]
	{425, L"Too Early"}, // [RFC8470]
	{426, L"Upgrade Required"}, // [RFC7231, Section 6.5.15]
	// 427 - Unassigned
	{428, L"Precondition Required"}, // [RFC6585]
	{429, L"Too Many Requests"}, // [RFC6585]
	// 430 - Unassigned
	{431, L"Request Header Fields Too Large"}, // [RFC6585]
	// 432 - 450 Unassigned
	{451, L"Unavailable For Legal Reasons"}, // [RFC7725]
	// 452 - 499 Unassigned
	{500, L"Internal Server Error"}, // [RFC7231, Section 6.6.1]
	{501, L"Not Implemented"}, // [RFC7231, Section 6.6.2]
	{502, L"Bad Gateway"}, // [RFC7231, Section 6.6.3]
	{503, L"Service Unavailable"}, // [RFC7231, Section 6.6.4]
	{504, L"Gateway Timeout"}, // [RFC7231, Section 6.6.5]
	{505, L"HTTP Version Not Supported"}, // [RFC7231, Section 6.6.6]
	{506, L"Variant Also Negotiates"}, // [RFC2295]
	{507, L"Insufficient Storage"}, // [RFC4918]
	{508, L"Loop Detected"}, // [RFC5842]
	// 509 Unassigned
	{510, L"Not Extended"}, // [RFC2774]
	{511, L"Network Authentication Required"}, // [RFC6585]
	// 512 - 599 Unassigned
};

static PCWCHAR get_status_code_description(int status_code) {
	const size_t len = sizeof(status_codes) / sizeof(t_status_code);
	for (size_t i = 0; i < len; ++i) {
		if (status_code == status_codes[i].code) {
			return status_codes[i].status;
		}
	}
	return L"Unknown";
}

CefRefPtr<CefResourceHandler> StreamResourceHandler::CreateFromString(
	int status_code,
	const CefString& mime_type,
	CefResponse::HeaderMap header_map,
	std::string& content) {
	auto r = new StreamResourceHandler(
		status_code,
		get_status_code_description(status_code),
		mime_type,
		header_map,
		CefStreamReader::CreateForData((void*)&content[0], content.size())
	);
	r->content = std::move(content);
	return r;
}

CefRefPtr<CefResourceHandler> StreamResourceHandler::CreateFromFile(
	int status_code,
	const CefString& mime_type,
	CefResponse::HeaderMap header_map,
	const CefString& filepath) {
	auto stream = CefStreamReader::CreateForFile(filepath);
	if (!stream) {
		return StreamResourceHandler::CreateError(500, "failed to read file");
	}
	return new CefStreamResourceHandler(
		status_code,
		get_status_code_description(status_code),
		mime_type,
		header_map,
		stream
	);
}

CefRefPtr<CefResourceHandler> StreamResourceHandler::CreateError(int status_code, const CefString& message) {
	std::string html("<html><body><h2>");
	html += message;
	html += "</h2></body></html>";
	return StreamResourceHandler::CreateFromString(status_code, "text/html", CefResponse::HeaderMap(), html);
}

Assets::Assets() : mime_map_initialized(false) {
}

void Assets::GetMimeJson(CefString& json) {
	json = R"JSON({".html":"text/html",".css":"text/css",".js":"text/javascript",".mjs":"text/javascript",".txt":"text/plain",".xml":"text/xml",".csv":"text/csv",".rss":"application/rss+xml",".json":"application/json",".ico":"image/vnd.microsoft.icon",".svg":"image/svg+xml",".jpg":"image/jpeg",".jpeg":"image/jpeg",".png":"image/png",".gif":"image/gif",".webp":"image/webp",".bmp":"image/bmp",".woff":"font/woff",".woff2":"font/woff2",".otf":"font/otf",".ttf":"font/ttf",".eot":"application/vnd.ms-fontobject",".webm":"video/webm",".mp4":"video/mp4",".ts":"video/mp2t",".mpg":"video/mpeg",".mpeg":"video/mpeg",".ogv":"video/ogg",".mov":"video/quicktime",".qt":"video/quicktime",".avi":"video/x-msvideo",".mp3":"audio/mpeg",".m4a":"audio/aac",".aac":"audio/aac",".mid":"audio/midi",".oga":"audio/ogg",".opus":"audio/opus",".wav":"audio/wav",".zip":"application/zip",".7z":"application/x-7z-compressed",".tar":"application/x-tar",".rar":"application/vnd.rar",".pdf":"application/pdf",".epub":"application/epub+zip",".gz":"application/gzip",".jar":"application/java-archive"})JSON";
}

CefRefPtr<CefResourceHandler> Assets::operator()(const CefString& path) {
	{
		std::lock_guard<std::mutex> lk(mtx_);
		if (!mime_map_initialized) {
			InitializeMimeMap();
		}
	}
	if (path.c_str()[path.size() - 1] == L'/') {
		std::wstring tmp(path.ToWString());
		tmp += L"index.html";
		return Get(tmp);
	}
	return  Get(path);
}

void Assets::InitializeMimeMap() {
	mime_map_initialized = true;
	mime_map_.clear();

	picojson::value v;
	{
		CefString json;
		GetMimeJson(json);
		const std::wstring w = json.ToWString();
		auto st = w.cbegin(), ed = w.cend();
		const std::string err = picojson::parse(v, st, ed);
		if (!err.empty() || !v.is<picojson::object>()) {
			return;
		}
	}
	const picojson::object& obj = v.get<picojson::object>();
	for (auto i = obj.cbegin(); i != obj.cend(); ++i) {
		mime_map_[i->first] = i->second.to_str();
	}
}

static int find_last_dot(const CefString& s) {
	int dot = -1;
	const CefString::char_type *ptr = s.c_str();
	for (size_t p = 0; p < s.length(); ++p) {
		if (ptr[p] == L'.') {
			dot = (int)p;
		}
	}
	return dot;
}

void Assets::GetMime(const CefString& path, CefString& mime) {
	PCWCHAR octetStream = L"application/octet-stream";
	const int last_dot = find_last_dot(path);
	if (last_dot == -1) {
		mime = octetStream;
		return;
	}

	std::lock_guard<std::mutex> lk(mtx_);
	const CefString ext(path.c_str() + last_dot, path.length()-last_dot, false);
	auto it = mime_map_.find(ext);
	if (it == mime_map_.cend()) {
		mime = octetStream;
		return;
	}
	mime = it->second;
	return;
}

void Assets::SetDefaultHeaders(CefResponse::HeaderMap& header_map) {
	header_map.insert(std::make_pair(L"Cross-Origin-Embedder-Policy", L"require-corp"));
	header_map.insert(std::make_pair(L"Cross-Origin-Opener-Policy", L"same-origin"));
}
