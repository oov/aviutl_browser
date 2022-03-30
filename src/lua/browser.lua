local P = {}

local function int8(x)
  return x%256
end

local function int16(x)
  local b2=x%256
  x=(x-b2)/256
  local b1=x%256
  return b1, b2
end

local function header(use_dir, use_devtools, path_len, userfile_len, tabid_len)
  local v1 = int8(2)
  local flag = 0
  if use_dir then
    flag = flag + 1
  end
  if use_devtools then
    flag = flag + 2
  end
  local f1 = int8(flag)
  local p1, p2 = int16(path_len)
  local u1, u2 = int16(userfile_len)
  local t1, t2 = int16(tabid_len)
  return string.char(v1, f1, p2, p1, u2, u1, t2, t1)
end

local bridge = require("bridge")
local exe_path = "\"" .. obj.getinfo("script_path") .. "exe\\aviutl_browser.exe\""

--- ブラウザーでコンテンツを表示し、内容を取得する
-- params には以下のようなデータを渡す
-- {
--   abc = "mycontent.abc", -- abcファイルをコンテンツとして読み込む場合は必須
--   dir = "mycontent", -- フォルダーをコンテンツとして読み込む場合は必須
--   tabid = "", -- 同じコンテンツをブラウザーの別タブで開く場合は識別用の文字列を指定する（省略可）
--   param = "", -- JavaScript に渡すデータがあるなら文字列で渡す（省略可）
--   userfile = "", -- ユーザーが選んだファイルがある場合は変数 file の内容をここへ指定する（省略可）
--   dev = false, -- 開発モードを有効にする場合は true
-- }
-- @param object params 渡すべき内容は上記を参照
-- @return bool, string 成功したかどうかと、JavaScript から渡された文字列
function P.execute(params)
  local path, use_dir
  if params.abc ~= nil then
    path, use_dir = params.abc, false
  elseif params.dir ~= nil then
    path, use_dir = params.dir, true
  end
  local tabid = params.tabid ~= nil and params.tabid or ""
  local userfile = params.userfile ~= nil and params.userfile or ""
  local param = params.param ~= nil and params.param or ""
  local r = bridge.call(exe_path, header(use_dir, params.dev, #path, #userfile, #tabid) .. path .. userfile .. tabid .. param, "w")
  return r:byte(1) ~= 0, r:sub(2)
end

return P
