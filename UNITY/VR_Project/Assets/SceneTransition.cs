using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class SceneTransition : MonoBehaviour
{
    [Header("Scene Transition Settings")]
    public string nextSceneName = "RoomScene"; // Name of your escape room scene
    public float triggerRadius = 2f; // How close player needs to be

    [Header("Visual Feedback")]
    public bool showDebugSphere = true; // Shows trigger area in scene view
    public Color debugColor = Color.cyan;

    [Header("Player Detection")]
    public string playerTag = "Player"; // Make sure your player has this tag

    private bool hasTriggered = false; // Prevent multiple triggers

    void Start()
    {
        // Create a trigger collider if one doesn't exist
        if (GetComponent<Collider>() == null)
        {
            SphereCollider trigger = gameObject.AddComponent<SphereCollider>();
            trigger.isTrigger = true;
            trigger.radius = triggerRadius;
        }

        Debug.Log("Tree Tunnel Teleporter Ready! Scene: " + nextSceneName);
    }

    void OnTriggerEnter(Collider other)
    {
        if (hasTriggered) return; // Prevent multiple triggers

        if (other.CompareTag(playerTag))
        {
            Debug.Log("Player entered tree tunnel! Loading: " + nextSceneName);
            hasTriggered = true;
            LoadNextScene();
        }
    }

    void LoadNextScene()
    {
        // Add a small delay for smooth transition
        Invoke("DoSceneTransition", 0.5f);
    }

    void DoSceneTransition()
    {
        SceneManager.LoadScene(nextSceneName);
    }

    // Show trigger area in scene view
    void OnDrawGizmosSelected()
    {
        if (showDebugSphere)
        {
            Gizmos.color = debugColor;
            Gizmos.DrawWireSphere(transform.position, triggerRadius);
        }
    }
}
