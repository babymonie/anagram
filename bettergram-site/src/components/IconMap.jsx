import {
  Languages, Mic, Sparkles, Tag, Image, Ghost, Clock, Type,
  Unlink2, MessageCircleOff, Bell, NotebookPen, EyeOff,
  StretchHorizontal, CircleOff, StarOff, BellOff, XCircle,
  UserCircle, Award, FileBox, Smile, Download, PictureInPicture2,
  CloudDownload, Globe, LayoutGrid, Unlock, Zap, ShieldCheck,
  Play, ChevronDown, Search, X, Check, Copy, ExternalLink,
  Menu, ArrowDown, Send, UserCheck, Database, Network, Eye, Palette,
} from 'lucide-react'

function GithubSvg({ size = 16, ...props }) {
  return (
    <svg width={size} height={size} viewBox="0 0 24 24" fill="currentColor" {...props}>
      <path d="M12 2C6.477 2 2 6.477 2 12c0 4.418 2.865 8.166 6.839 9.489.5.092.682-.217.682-.482 0-.237-.009-.868-.013-1.703-2.782.604-3.369-1.342-3.369-1.342-.454-1.155-1.11-1.463-1.11-1.463-.908-.62.069-.608.069-.608 1.003.07 1.531 1.03 1.531 1.03.892 1.529 2.341 1.087 2.91.832.092-.647.35-1.088.636-1.338-2.22-.253-4.555-1.11-4.555-4.943 0-1.091.39-1.984 1.03-2.682-.103-.253-.447-1.27.098-2.646 0 0 .84-.269 2.75 1.025A9.578 9.578 0 0 1 12 6.836a9.59 9.59 0 0 1 2.504.337c1.909-1.294 2.747-1.025 2.747-1.025.547 1.376.203 2.394.1 2.646.64.698 1.028 1.591 1.028 2.682 0 3.841-2.337 4.687-4.565 4.935.359.309.678.919.678 1.852 0 1.336-.012 2.415-.012 2.743 0 .267.18.578.688.48C19.138 20.163 22 16.418 22 12c0-5.523-4.477-10-10-10z" />
    </svg>
  )
}

const ICONS = {
  Languages, Mic, Sparkles, Tag, Image, Ghost, Clock, Type,
  LinkOff: Unlink2, MessageCircleOff, Bell, NotebookPen, EyeOff,
  StretchHorizontal, CircleOff, StarOff, BellOff, XCircle,
  UserCircle, Award, FileBox, Smile, Download, PictureInPicture2,
  CloudDownload, Globe, LayoutGrid, Unlock, Zap, ShieldCheck,
  Play, ChevronDown, Search, X, Check, Copy, ExternalLink,
  Github: GithubSvg, Menu, ArrowDown, Send, UserCheck, Database, Network, Eye, Palette,
}

export function Icon({ name, size = 16, ...props }) {
  const Component = ICONS[name]
  if (!Component) return null
  return <Component size={size} {...props} />
}

export default ICONS
