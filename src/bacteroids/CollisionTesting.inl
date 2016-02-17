
#define BUCKETED_COLLISION 1
#if defined(BUCKETED_COLLISION)
    size_t_32 num_objects = m_objects.Size();

    size_t_32 q[5] = {0};

    for (size_t_32 i = 0; i < num_objects; i++)
    {
        GameObject *o = m_objects[i];
        vec2f p = o->GetPosition();
        float r = o->GetRadius();

        if (p.x + r < 0.0f && p.y + r < 0.0f)
        {
            size_t_32 q0 = q[0]++;
            size_t_32 q1 = q[1]++;
            size_t_32 q2 = q[2]++;
            size_t_32 q3 = q[3]++;
            size_t_32 q4 = q[4]++;
            m_quadTree[q4] = m_quadTree[q3];
            m_quadTree[q3] = m_quadTree[q2];
            m_quadTree[q2] = m_quadTree[q1];
            m_quadTree[q1] = m_quadTree[q0];
            m_quadTree[q0] = o;
        }
        else if (p.x + r < 0.0f && p.y - r > 0.0f)
        {
            size_t_32 q1 = q[1]++;
            size_t_32 q2 = q[2]++;
            size_t_32 q3 = q[3]++;
            size_t_32 q4 = q[4]++;
            m_quadTree[q4] = m_quadTree[q3];
            m_quadTree[q3] = m_quadTree[q2];
            m_quadTree[q2] = m_quadTree[q1];
            m_quadTree[q1] = o;
        }
        else if (p.x - r > 0.0f && p.y + r < 0.0f)
        {
            size_t_32 q2 = q[2]++;
            size_t_32 q3 = q[3]++;
            size_t_32 q4 = q[4]++;
            m_quadTree[q4] = m_quadTree[q3];
            m_quadTree[q3] = m_quadTree[q2];
            m_quadTree[q2] = o;
        }
        else if (p.x - r > 0.0f && p.y - r > 0.0f)
        {
            size_t_32 q3 = q[3]++;
            size_t_32 q4 = q[4]++;
            m_quadTree[q4] = m_quadTree[q3];
            m_quadTree[q3] = o;
        }
        else
        {
            size_t_32 q4 = q[4]++;
            m_quadTree[q4] = o;
        }
    }

    for (size_t_32 k = 0, i = 0; k < 5; k++)
    {
        for (; i < q[k]; i++)
        {
            GameObject *obj1 = m_quadTree[i];
            vec2f p1 = obj1->GetPosition();
            float r1 = obj1->GetRadius();

            for (size_t_32 j = i + 1; j < q[k]; j++)
            {
                GameObject *obj2 = m_quadTree[j];
                vec2f p2 = obj2->GetPosition();
                float r2 = obj2->GetRadius();

                vec2f from2To1 = (p1 - p2);
                float dist = r1 + r2 - from2To1.Length();

                if (dist > 0.0f)
                {
                    DoCollision(obj1, obj2, from2To1, dist);
                    DoCollision(obj2, obj1, -from2To1, dist);
                }
            }

            if (k == 4) continue;

            for (size_t_32 j = q[3]; j < q[4]; j++)
            {
                GameObject *obj2 = m_quadTree[j];
                vec2f p2 = obj2->GetPosition();
                float r2 = obj2->GetRadius();

                vec2f from2To1 = (p1 - p2);
                float dist = r1 + r2 - from2To1.Length();

                if (dist > 0.0f)
                {
                    DoCollision(obj1, obj2, from2To1, dist);
                    DoCollision(obj2, obj1, -from2To1, dist);
                }
            }
        }
    }
#else
    size_t_32 num_objects = m_objects.Size();

    for (size_t_32 i = 0; i < num_objects; i++)
    {
        GameObject *obj1 = m_objects[i];
        vec2f p1 = obj1->GetPosition();
        float r1 = obj1->GetRadius();

        for (size_t_32 j = i + 1; j < num_objects; j++)
        {
            GameObject *obj2 = m_objects[j];
            vec2f p2 = obj2->GetPosition();
            float r2 = obj2->GetRadius();

            vec2f from2To1 = (p1 - p2);
            float dist = r1 + r2 - from2To1.Length();

            if (dist > 0.0f)
            {
                DoCollision(obj1, obj2, from2To1, dist);
                DoCollision(obj2, obj1, -from2To1, dist);
            }
        }
    }
#endif // BUCKETED_COLLISION
