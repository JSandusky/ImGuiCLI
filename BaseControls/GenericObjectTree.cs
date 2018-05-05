using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ImGuiCLI;
using System.Collections;

namespace ImGuiControls
{
    public class GenericObjectTree
    {
        public interface ISelection
        {
            bool IsSelected(object obj);
            void Select(object obj, bool additive);
            void Deselect(object obj);
            void Drop(object onto, string key);
        }

        ReflectionCache cache_;
        public List<object> RootObjects { get; set; } = new List<object>();

        public Func<object, string> StringConverter;
        public Func<object, string> DragConverter;
        public Action<object> ContextMenu;
        public ISelection Selection;

        public GenericObjectTree(ReflectionCache cache)
        {
            cache_ = cache;
        }

        public void DrawAsWindow(string title)
        {
            if (ImGuiCli.Begin(title, ImGuiWindowFlags_.ResizeFromAnySide))
                Draw();
            ImGuiCli.End();
        }

        public void DrawAsDock(string title)
        {
            if (ImGuiDock.BeginDock(title))
                Draw();
            ImGuiDock.EndDock();
        }

        public void Draw()
        {
            Draw(RootObjects, 1);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="obj">Object the node is for</param>
        /// <param name="text">Desired text, not required to use</param>
        /// <param name="isSelected">Selection status of the node</param>
        protected virtual void DrawNodeObject(object obj, string text, bool isSelected)
        {
            // Insert an image here, preceded by an ImGuiCli.Sameline
            ImGuiCli.SameLine();
            ImGuiCli.Selectable(text, isSelected);
            if (Selection != null && ImGuiCli.IsItemClicked(0))
            {
                if (ImGuiIO.KeyCtrl)
                {
                    if (isSelected)
                        Selection.Deselect(obj);
                    else
                        Selection.Select(obj, true);
                }
                else
                    Selection.Select(obj, false);
            }
            else if (ImGuiCli.IsItemClicked(1) && ContextMenu != null)
                ImGuiCli.OpenPopup("###tree_ctx");

            if (ImGuiCli.BeginPopup("###tree_ctx", ImGuiWindowFlags_.None))
            {
                ContextMenu(obj);
                ImGuiCli.EndPopup();
            }

            if (DragConverter != null)
            {
                if (!ImGuiCli.IsPopupOpen() && ImGuiCli.BeginDragDropSource())
                {
                    ImGuiCli.SetDragDropPayload("U_TREE", DragConverter(obj));
                    ImGuiCli.Text(text);
                    ImGuiCli.EndDragDropSource();
                }
                else if (ImGuiCli.BeginDragDropTarget())
                {
                    string data = null;
                    if (ImGuiCli.AcceptDragDropPayload("U_TREE", ref data))
                    {
                        Selection.Drop(obj, data);
                    }
                }
            }
        }

        void Draw(IList list, int depth)
        {
            for (int i = 0; i < list.Count; ++i)
            {
                int id = unchecked(depth << 15 + i);
                ImGuiCli.PushID(id);
                object obj = list[i];
                var listFields = cache_.GetAlphabetical(obj.GetType()).Where(f => typeof(IList).IsAssignableFrom(f.Type)).ToArray();

                bool isLeaf = listFields == null || listFields.Length == 0;
                if (listFields.Length == 1)
                    isLeaf |= (listFields[0].GetValue(obj) as IList).Count == 0;

                string text = StringConverter != null ? StringConverter(obj) : obj.ToString();
                
                ImGuiTreeNodeFlags_ nodeFlags = isLeaf ? ImGuiTreeNodeFlags_.Leaf : 0;
                bool isSelected = Selection != null ? Selection.IsSelected(obj) : false;
                if (isSelected)
                    nodeFlags |= ImGuiTreeNodeFlags_.Selected;

                bool open = ImGuiCli.TreeNodeEx("##" + id, nodeFlags);
                DrawNodeObject(obj, text, isSelected);

                if (open && listFields != null)
                {
                    if (listFields.Length == 1)
                    {
                        Draw(listFields[0].GetValue(obj) as IList, unchecked(depth * 36 + i));
                    }
                    else
                    {
                        // poly tree
                        for (int listIdx = 0; listIdx < listFields.Length; ++listIdx)
                        {
                            if (ImGuiCli.TreeNode(listFields[listIdx].DisplayName))
                            {
                                Draw(listFields[listIdx].GetValue(obj) as IList, depth + 1);
                                ImGuiCli.TreePop();
                            }
                        }
                    }
                    ImGuiCli.TreePop();
                }
                ImGuiCli.PopID();
            }
        }
    }
}
