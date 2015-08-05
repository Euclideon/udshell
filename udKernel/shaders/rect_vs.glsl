attribute vec2 a_position;
varying vec2 v_texcoord;
varying vec2 v_texcoord2;
uniform vec4 u_rect;
uniform vec4 u_textureScale;
uniform vec4 u_textureScale2;
void main()
{
  v_texcoord = u_textureScale.xy + a_position*u_textureScale.zw;
  v_texcoord2 = u_textureScale2.xy + a_position*u_textureScale2.zw;
  vec2 pos = u_rect.xy + a_position*u_rect.zw;
  gl_Position = vec4(pos.x, pos.y, 1, 1);
}
