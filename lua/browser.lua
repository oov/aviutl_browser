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

local function header(version, use_dir, use_devtools, path_len, userfile_len)
  local v1 = int8(version)
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
  return string.char(v1, f1, p2, p1, u2, u1, 0, 0)
end

local bridge = require("bridge")
local exe_path = "\"" .. obj.getinfo("script_path") .. "exe\\aviutl_browser.exe\""

--- �u���E�U�[�ŃR���e���c��\�����A���e���擾����
-- params �ɂ͈ȉ��̂悤�ȃf�[�^��n��
-- {
--   abc = "mycontent.abc", -- abc�t�@�C�����R���e���c�Ƃ��ēǂݍ��ޏꍇ�͕K�{
--   dir = "mycontent", -- �t�H���_�[���R���e���c�Ƃ��ēǂݍ��ޏꍇ�͕K�{
--   param = "", -- JavaScript �ɓn���f�[�^������Ȃ當����œn��
--   userfile = "", -- ���[�U�[���I�񂾃t�@�C��������ꍇ�͕ϐ� file �̓��e�������֎w�肷��
--   dev = false, -- �J�����[�h��L���ɂ���ꍇ�� true
-- }
-- @param object params �n���ׂ����e�͏�L���Q��
-- @return bool, string �����������ǂ����ƁAJavaScript ����n���ꂽ������
function P.execute(params)
  local path, use_dir
  if params.abc ~= nil then
    path, use_dir = params.abc, false
  elseif params.dir ~= nil then
    path, use_dir = params.dir, true
  end
  local userfile = params.userfile ~= nil and params.userfile or ""
  local param = params.param ~= nil and params.param or ""
  local r = bridge.call(exe_path, header(1, use_dir, params.dev, #path, #userfile) .. path .. userfile .. param, "w")
  return r:byte(1) ~= 0, r:sub(2)
end

return P