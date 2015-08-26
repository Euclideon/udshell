#if defined(USE_GLES)
#extension GL_EXT_frag_depth : require
precision mediump float;
#endif
varying vec2 v_texcoord;
uniform sampler2D u_texture;
uniform sampler2D u_zbuffer;
void main()
{
  float depthSample = texture2D(u_zbuffer, v_texcoord).x;
  float z = (gl_DepthRange.diff*depthSample + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
#if defined(USE_GLES)
  gl_FragDepthEXT = z;
#else
  gl_FragDepth = z;
#endif
  gl_FragColor = texture2D(u_texture, v_texcoord);
}
